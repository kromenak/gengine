#include "SidneySearch.h"

#include "Actor.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "GameProgress.h"
#include "IniParser.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UITextInput.h"
#include "UIUtil.h"

namespace
{
    // Crazy enough, when we show search results, we've actually got to parse some HTML here! And not particularly well formed HTML at that!
    // These structs will help to parse the HTML tree and store an in-memory representation.

    // An HTML attribute is a key/value pair associated with an HTML element.
    struct HtmlAttribute
    {
        std::string key;
        std::string value;
    };

    // An HTML element typically consists of open and close tags, plus contents in between those tags.
    // The open tag can also have zero or more HTML attributes.
    struct HtmlElement
    {
        // Either stores an HTML tag OR text data in leaf nodes.
        std::string tagOrData;

        // If this is a tag, any attributes associated with the tag.
        std::vector<HtmlAttribute> attributes;

        // An ordered list of contents inside the HTML element.
        // These are themselves elements, giving us the tree structure.
        std::vector<HtmlElement> contents;
    };

    void ParseHtml(TextAsset* html, HtmlElement& root)
    {
        // We need a valid asset pointer.
        if(html == nullptr) { return; }

        // Get the actual text data from the asset.
        char* text = reinterpret_cast<char*>(html->GetText());
        uint32_t textLength = html->GetTextLength();
        if(textLength == 0)
        {
            return;
        }

        // Keeps track of where we are in the HTML tree as we parse it.
        std::vector<HtmlElement*> parseStack;

        // Holds current content string (outside of tags) that is being built.
        std::string content;

        // Iterate until we reach the end of the HTML text, parsing each character in turn.
        int textIndex = 0;
        while(textIndex < html->GetTextLength())
        {
            // Read a character.
            char c = text[textIndex];
            ++textIndex;

            // Skip line breaks.
            if(c == '\n' || c == '\r') { continue; }

            // We encountered a tag, but the content isn't empty.
            // Add it to the current element's contents before continuing.
            if(c == '<' && !content.empty())
            {
                StringUtil::TrimWhitespace(content); // make sure it isn't just whitespace.
                if(!content.empty())
                {
                    HtmlElement textElement;
                    textElement.tagOrData = content;
                    parseStack.back()->contents.push_back(textElement);

                    // Added the content; clear working string for next time.
                    content.clear();
                }
            }

            // This is a tag - figure out what the tag is.
            if(c == '<')
            {
                // Read in the entire string.
                std::string tagStr;
                while(textIndex < textLength && text[textIndex] != '>')
                {
                    tagStr.push_back(text[textIndex]);
                    ++textIndex;
                }

                // Skip past the close tag.
                ++textIndex;

                // Split by spaces.
                std::vector<std::string> tokens = StringUtil::Split(tagStr, ' ', true);
                if(tokens.empty())
                {
                    continue; // empty brackets, like <>? Ignore?
                }

                // If first char of first token is a /, then this is closing some other tag (e.g. /FONT /A etc)
                if(tokens[0][0] == '/')
                {
                    // Get rid of slash char.
                    tokens[0].erase(0, 1);
                    //printf("Found close tag %s\n", tokens[0].c_str());

                    // But then again, some web pages inconsistently use close tags on elements we usually expect no close tag on...
                    // Just ignore them in that case.
                    if(StringUtil::EqualsIgnoreCase(tokens[0], "P") ||
                       StringUtil::EqualsIgnoreCase(tokens[0], "BR") ||
                       StringUtil::EqualsIgnoreCase(tokens[0], "LI") ||
                       StringUtil::EqualsIgnoreCase(tokens[0], "HR") ||
                       StringUtil::EqualsIgnoreCase(tokens[0], "IMG"))
                    {
                        continue;
                    }

                    // The close tag should match whatever's on the back of the parse stack.
                    // If not, it means we have a close tag with a mistmatched open tag.
                    if(StringUtil::EqualsIgnoreCase(parseStack.back()->tagOrData, tokens[0]))
                    {
                        // Closed this tag, so pop it off the parse stack.
                        parseStack.pop_back();
                    }
                    else
                    {
                        printf("HTML: close tag %s doesn't match with an open tag in %s. Ignoring!\n", tokens[0].c_str(), html->GetName().c_str());
                    }
                    continue;
                }

                // Ok, this is an open tag.
                HtmlElement element;
                element.tagOrData = tokens[0];

                // Remaining tokens are attributes.
                for(int i = 1; i < tokens.size(); ++i)
                {
                    // Each attribute should be either a keyword or a key/value pair (e.g. SRC="BLAH")
                    std::vector<std::string> attributeTokens = StringUtil::Split(tokens[i], '=', true);

                    // Make sure quotes are removed from around attributes.
                    for(std::string& attributeToken : attributeTokens)
                    {
                        StringUtil::Trim(attributeToken, '\"');
                    }

                    if(attributeTokens.empty())
                    {
                        continue;
                    }
                    else if(attributeTokens.size() == 1)
                    {
                        element.attributes.emplace_back();
                        element.attributes.back().key = attributeTokens[0];
                    }
                    else
                    {
                        element.attributes.emplace_back();
                        element.attributes.back().key = attributeTokens[0];
                        element.attributes.back().value = attributeTokens[1];
                    }
                }

                // Ok, we've fully parsed this open tag. Figure out what to do with it now.

                // The HTML tag is special: it's the root of the HTML. Save it as such.
                if(StringUtil::EqualsIgnoreCase(element.tagOrData, "HTML"))
                {
                    assert(parseStack.empty());
                    root = element;
                    parseStack.push_back(&root);
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "IMG") ||
                        StringUtil::EqualsIgnoreCase(element.tagOrData, "HR") ||
                        StringUtil::EqualsIgnoreCase(element.tagOrData, "P") ||
                        StringUtil::EqualsIgnoreCase(element.tagOrData, "BR") ||
                        StringUtil::EqualsIgnoreCase(element.tagOrData, "LI"))
                {
                    // These tags do not have close tags.
                    // They don't have children, so no need to put them on the parse stack.
                    parseStack.back()->contents.push_back(element);
                }
                else // a normal tag that should have a close tag later
                {
                    // Add to contents of the current parent element.
                    parseStack.back()->contents.push_back(element);

                    // We are now *inside* this element.
                    parseStack.push_back(&parseStack.back()->contents.back());
                }
            }
            else // must be content?
            {
                content.push_back(c);
            }
        }
        assert(parseStack.empty());
    }
}

void SidneySearch::Init(Actor* parent)
{
    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Search");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });
    
    // Add menu bar.
    SidneyUtil::CreateMenuBar(mRoot, "SEARCH", 100.0f);

    // Add search bar (top area).
    {
        // Background w/ border.
        UINineSlice* searchBarPanel = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::VeryTransBgColor));
        searchBarPanel->GetRectTransform()->SetPivot(0.0f, 1.0f);
        searchBarPanel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        searchBarPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -88.0f);
        searchBarPanel->GetRectTransform()->SetSizeDelta(520.0f, 48.0f);
        
        // Reset button.
        SidneyButton* resetButton = SidneyUtil::CreateSmallButton(searchBarPanel->GetOwner());
        resetButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        resetButton->GetRectTransform()->SetAnchoredPosition(14.0f, -17.0f);
        resetButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Reset"));
        resetButton->SetPressCallback([this](){
            OnResetButtonPressed();
        });

        // Search button.
        SidneyButton* searchButton = SidneyUtil::CreateSmallButton(searchBarPanel->GetOwner());
        searchButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        searchButton->GetRectTransform()->SetAnchoredPosition(426.0f, -17.0f);
        searchButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Search"));
        searchButton->SetPressCallback([this](){
            OnSearchButtonPressed();
        });

        // Text input field.
        UINineSlice* searchInputPanel = UIUtil::NewUIActorWithWidget<UINineSlice>(searchBarPanel->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
        searchInputPanel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        searchInputPanel->GetRectTransform()->SetAnchoredPosition(100.0f, -17.0f);
        searchInputPanel->GetRectTransform()->SetSizeDelta(320.0f, 15.0f);

        mTextInput = searchInputPanel->GetOwner()->AddComponent<UITextInput>();
        mTextInput->SetFont(gAssetManager.LoadFont("F_TIMES.FON"));
        mTextInput->SetVerticalAlignment(VerticalAlignment::Bottom);
        mTextInput->SetText("");

        // Create text input field caret.
        UIImage* caretImage = UIUtil::NewUIActorWithWidget<UIImage>(searchInputPanel->GetOwner());
        caretImage->SetTexture(&Texture::White);
        caretImage->GetRectTransform()->SetAnchor(AnchorPreset::LeftStretch, false);
        caretImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        caretImage->GetRectTransform()->SetSizeDelta(1.0f, 0.0f);

        mTextInput->SetCaret(caretImage);
        mTextInput->SetCaretBlinkInterval(0.5f);
    }

    // Add navigation bar (bottom area).
    {
        // Background.
        UINineSlice* navBarPanel = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::VeryTransBgColor));
        navBarPanel->GetRectTransform()->SetPivot(0.0f, 1.0f);
        navBarPanel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        navBarPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -390.0f);
        navBarPanel->GetRectTransform()->SetSizeDelta(520.0f, 28.0f);

        // Back button.
        SidneyButton* backButton = SidneyUtil::CreateSmallButton(navBarPanel->GetOwner());
        backButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        backButton->GetRectTransform()->SetAnchoredPosition(63.0f, -8.0f);
        backButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Back"));
        backButton->SetPressCallback([](){
            printf("Back\n");
        });

        // Forward button.
        SidneyButton* fwdButton = SidneyUtil::CreateSmallButton(navBarPanel->GetOwner());
        fwdButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        fwdButton->GetRectTransform()->SetAnchoredPosition(155.0f, -8.0f);
        fwdButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Forward"));
        fwdButton->SetPressCallback([](){
            printf("Forward\n");
        });
    }

    // Add search results web page area (middle).
    {
        UINineSlice* resultsPanel = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::VeryTransBgColor));
        resultsPanel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        resultsPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -136.0f);
        resultsPanel->GetRectTransform()->SetSizeDelta(520.0f, 254.0f);
        mWebPageRoot = resultsPanel->GetOwner();

        mWebPageWidgetsCanvas = UIUtil::NewUIActorWithCanvas(resultsPanel->GetOwner(), -1);
        mWebPageWidgetsCanvas->SetMasked(true);
        mWebPageWidgetsCanvas->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mWebPageWidgetsCanvas->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        mWebPageWidgetsCanvas->GetRectTransform()->SetSizeDelta(-4.0f, 0.0f);

        /*
        UIImage* resultsBackgroundImage = UIUtil::NewUIActorWithWidget<UIImage>(mRoot);
        resultsBackgroundImage->SetTexture(&Texture::Black);
        resultsBackgroundImage->SetColor(Color32(0, 0, 0, 128)); // Black Semi-Transparent
        resultsBackgroundImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        resultsBackgroundImage->GetRectTransform()->SetAnchoredPosition(60.0f, -136.0f);
        resultsBackgroundImage->GetRectTransform()->SetSizeDelta(520.0f, 254.0f);

        //TODO: So there is a big undertaking here to implement a whole HTML parsing system.
        //TODO: For the moment though, I'll just use a label to display a simple result output.
        mResultsTextBuffer = resultsBackgroundImage->GetOwner()->AddComponent<UILabel>();
        mResultsTextBuffer->SetFont(gAssetManager.LoadFont("F_TIMES.FON"));
        mResultsTextBuffer->SetHorizonalAlignment(HorizontalAlignment::Left);
        mResultsTextBuffer->SetVerticalAlignment(VerticalAlignment::Top);
        */

        // Web page area is disabled by default. It enables when you search for something.
        mWebPageRoot->SetActive(false);
    }

    // Read in all the search terms and correlated pages.
    TextAsset* textFile = gAssetManager.LoadText("SIDSEARCH.TXT");
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);

    // Each section's name correlates to the associated HTML page.
    // Each section then has a single line ("text") with a comma-separated list of terms.
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        for(auto& line : section.lines)
        {
            if(line.entries[0].key == "text")
            {
                std::vector<std::string> terms = StringUtil::Split(line.entries[0].value, ',');
                for(auto& term : terms)
                {
                    mSearchTerms[term] = section.name;
                }
            }
        }
    }

    // Hide by default.
    Hide();
}

void SidneySearch::Show()
{
    mRoot->SetActive(true);
    mTextInput->Focus();
}

void SidneySearch::Hide()
{
    mRoot->SetActive(false);
}

void SidneySearch::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }

    // If enter is pressed, act like a search was done.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
    {
        OnSearchButtonPressed();
    }
}

void SidneySearch::ShowWebPage(const std::string& pageName)
{
    // First, make sure any previous web page has been cleared.
    ClearWebPage();

    // Load the HTML text.
    TextAsset* html = gAssetManager.LoadText(pageName, AssetScope::Scene);

    // Parse the HTML text to an in-memory HTML tree representation.
    HtmlElement root;
    ParseHtml(html, root);
    
    // At this point, we've got a fully fleshed out HTML structure.
    // Next, we need to create the UI elements to graphically reflect that.

    // To render the HTML: all leaf nodes (ones with no children) are renderable.
    // Parent nodes to the leaf nodes may modify how a leaf node renders (e.g. if leaf node parent is a <B> tag, the leaf node is bolded).

    // As we parse the HTML tree to render it, we need to keep track of the current "path" through the tree.
    // We start at the root, at child index 0.
    std::vector<HtmlElement*> parseStack;
    std::vector<int> parseIndexes;
    parseStack.push_back(&root);
    parseIndexes.push_back(0);

    // Keep track of position to render at, from top-left.
    Vector2 resultsPos(0.0f, 0.0f);

    // Parse the HTML, starting at the root, in a depth-first way.
    // This *could* work recursively, but the algorithm is done iteratively here.
    while(!parseStack.empty())
    {
        // Each iteration of this loop parses the contents of whichever HtmlElement is at the back of the parse stack.
        HtmlElement* parentElement = parseStack.back();
        int& index = parseIndexes.back();

        // If we've parsed all the contents for this element, pop it off the parse stack - we're done with it. 
        if(index >= parentElement->contents.size())
        {
            parseStack.pop_back();
            parseIndexes.pop_back();
        }
        else // still a valid index...
        {
            // Grab the child element at the current index.
            HtmlElement& element = parseStack.back()->contents[index];

            // Increment the index for next iteration of the while loop.
            ++index;

            // Check if this is a leaf node in the HTML tree.
            // If so, it means this is something we can render! Otherwise, we need to keep searching.
            bool isLeaf = element.contents.empty();
            if(!isLeaf)
            {
                // We don't render anything within the HEAD element, so we can ignore that entire subtree.
                if(StringUtil::EqualsIgnoreCase(element.tagOrData, "HEAD"))
                {
                    continue;
                }

                // If not a leaf, we need to explore this element's children.
                // Push this element onto the parse stack, along with a new parse index of 0.
                parseStack.push_back(&element);
                parseIndexes.push_back(0);
            }
            else
            {
                // This is a leaf node, so we will render it.
                // However, we need to look at the current stack of tags to see what render mods are applied.

                // For text rendering, font parameters.
                int fontSize = 0;
                bool bold = false;
                bool italic = false;

                // For links, what page to link to.
                std::string link;

                // If true, we put a bullet point in front of this item.
                bool listItem = false;

                // Iterate the parse stack to see what tags this leaf node is nested within.
                // Depending on the tags, it may alter how this leaf node renders (e.g. if we're nested within a B tag, we use a bold font).
                for(int i = 0; i < parseStack.size(); ++i)
                {
                    if(StringUtil::EqualsIgnoreCase(parseStack[i]->tagOrData, "Font"))
                    {
                        int fontSizeDiff = std::atoi(parseStack[i]->attributes[0].value.c_str());
                        fontSize += fontSizeDiff;
                    }
                    else if(StringUtil::EqualsIgnoreCase(parseStack[i]->tagOrData, "B"))
                    {
                        bold = true;
                    }
                    else if(StringUtil::EqualsIgnoreCase(parseStack[i]->tagOrData, "I"))
                    {
                        italic = true;
                    }
                    else if(StringUtil::EqualsIgnoreCase(parseStack[i]->tagOrData, "A"))
                    {
                        link = parseStack[i]->attributes[0].value;
                    }
                }

                // Determine what font to use.
                std::string font = (fontSize == 2 ? "F_TIMES_2.FON" : "F_TIMES.FON");
                if(bold && italic)
                {
                    printf("HTML: unsupported text format bold AND italic...\n");
                }
                else if(bold)
                {
                    font = (fontSize == 2 ? "F_TIMES_B_2.FON" : "F_TIMES_B.FON");
                }
                else if(italic)
                {
                    if(fontSize != 0)
                    {
                        printf("HTML: unsupported text format (italic font size %i)\n", fontSize);
                    }
                    font = "F_TIMES_I.FON";
                }
                //TODO: Assets exist for underlined font (normal and +2) and bold+underlined font (normal and +2). But never used in-game?

                // Links always use the link variant of the font.
                if(!link.empty())
                {
                    font = "F_TIMES_L.FON";
                }

                // Handle rendering the leaf data.
                if(StringUtil::EqualsIgnoreCase(element.tagOrData, "P"))
                {
                    // This just breaks to the next line.
                    resultsPos.x = 0.0f;
                    resultsPos.y -= 10.0f;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "BR"))
                {
                    // Do nothing?
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "LI"))
                {
                    UIImage* bulletImage = UIUtil::NewUIActorWithWidget<UIImage>(mWebPageWidgetsCanvas->GetOwner());
                    bulletImage->SetTexture(gAssetManager.LoadTexture("SIDNEYBULLET.BMP", AssetScope::Scene), true);
                    bulletImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    bulletImage->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    mWebPageWidgets.push_back(bulletImage);

                    resultsPos.x += bulletImage->GetRectTransform()->GetRect().width + 2.0f;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "HR"))
                {
                    UIImage* hrImage = UIUtil::NewUIActorWithWidget<UIImage>(mWebPageWidgetsCanvas->GetOwner());
                    hrImage->SetTexture(gAssetManager.LoadTexture("HORIZONTALRULE.BMP", AssetScope::Scene), true);
                    hrImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    hrImage->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    mWebPageWidgets.push_back(hrImage);

                    resultsPos.x = 0.0f;
                    resultsPos.y -= hrImage->GetRectTransform()->GetRect().height;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "IMG"))
                {
                    UIImage* image = UIUtil::NewUIActorWithWidget<UIImage>(mWebPageWidgetsCanvas->GetOwner());
                    image->SetTexture(gAssetManager.LoadTexture(element.attributes[0].value, AssetScope::Scene), true);
                    image->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    image->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    mWebPageWidgets.push_back(image);

                    resultsPos.x = 0.0f;
                    resultsPos.y -= image->GetRectTransform()->GetRect().height;
                }
                else // text or text link?
                {
                    // Create a label with the appropriate font and text.
                    UILabel* label = UIUtil::NewUIActorWithWidget<UILabel>(mWebPageWidgetsCanvas->GetOwner());
                    label->SetFont(gAssetManager.LoadFont(font));
                    label->SetText(element.tagOrData);
                    label->SetVerticalAlignment(VerticalAlignment::Top);
                    label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
                    label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    label->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    label->GetRectTransform()->SetSizeDeltaX(520.0f);
                    mWebPageWidgets.push_back(label);

                    // If this is a link, also make this bit of text act as a button.
                    if(!link.empty())
                    {
                        UIButton* button = label->GetOwner()->AddComponent<UIButton>();
                        button->SetPressCallback([link, this](UIButton* button){
                            ShowWebPage(link);
                        });
                        mWebPageWidgets.push_back(button);
                    }

                    resultsPos.x = 0.0f;
                    resultsPos.y -= label->GetTextHeight();
                }

                /*
                std::string toRender;
                for(int i = 0; i < parseStack.size(); ++i)
                {
                    toRender += parseStack[i]->tagOrData + " > ";
                }
                printf("Render HTML %s\n", toRender.c_str());
                */
            }
        }
    }

    // Make sure the web page root UI element is active, so you can see everything!
    mWebPageRoot->SetActive(true);
}

void SidneySearch::ClearWebPage()
{
    // Destroy all widgets that were created for this web page.
    for(UIWidget* widget : mWebPageWidgets)
    {
        widget->GetOwner()->Destroy();

        // Due to a bug/complexity (widget destructors can't safely remove themselves from their parent Canvases) - we must manually remove from canvas here.
        //TODO: Would be great for Widgets to be able to handle this internally...
        mWebPageWidgetsCanvas->RemoveWidget(widget);
    }
    mWebPageWidgets.clear();

    mWebPageRoot->SetActive(false);
}

void SidneySearch::OnSearchButtonPressed()
{
    auto it = mSearchTerms.find(mTextInput->GetText());
    if(it != mSearchTerms.end())
    {
        ShowWebPage(it->second);
        
        // Searching certain terms leads to point increases and flag setting.
        //TODO: Figure out how to make this data-driven!
        if(StringUtil::EqualsIgnoreCase(it->second, "Vampire.html"))
        {
            gGameProgress.ChangeScore("e_sidney_search_vampires");
            gGameProgress.SetFlag("Vampire");
        }
    }
    else
    {
        //mResultsTextBuffer->SetText(SidneyUtil::GetSearchLocalizer().GetText("NotFound"));
    }
}

void SidneySearch::OnResetButtonPressed()
{
    // Reset clears any text input and hides the current web page.
    mTextInput->Clear();
    ClearWebPage();
}