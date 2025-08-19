#include "SidneySearch.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "Console.h"
#include "Font.h"
#include "GameProgress.h"
#include "IniParser.h"
#include "InputManager.h"
#include "LayerManager.h"
#include "SidneyButton.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIScrollRect.h"
#include "UITextInput.h"
#include "UIUtil.h"

namespace
{
    const float kWebpageWidth = 520.0f;
    const float kScrollbarWidth = 8.0f;
    const float kWebpageContentsWidth = kWebpageWidth - kScrollbarWidth;

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

            // Treat line breaks in special ways depending on context.
            if(c == '\n' || c == '\r')
            {
                if(!content.empty() && content.back() != ' ')
                {
                    c = ' ';
                }
                else
                {
                    continue;
                }
            }

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
    // This menu bar only has one dropdown (web page history) that is oddly positioned...
    mMenuBar.Init(mRoot, SidneyUtil::GetSearchLocalizer().GetText("ScreenName"));
    mMenuBar.SetFirstDropdownPosition(405.0f);
    mMenuBar.AddDropdown("History");

    // Create popup.
    mPopup = new SidneyPopup(mRoot);

    // Add search bar (top area).
    {
        // Background w/ border.
        UINineSlice* searchBarPanel = UI::CreateWidgetActor<UINineSlice>("SearchBarBox", mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        searchBarPanel->GetRectTransform()->SetPivot(0.0f, 1.0f);
        searchBarPanel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        searchBarPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -88.0f);
        searchBarPanel->GetRectTransform()->SetSizeDelta(kWebpageWidth, 48.0f);

        // Reset button.
        SidneyButton* resetButton = SidneyUtil::CreateSmallButton(searchBarPanel->GetOwner());
        resetButton->SetName("ResetButton");
        resetButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        resetButton->GetRectTransform()->SetAnchoredPosition(14.0f, -17.0f);
        resetButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Reset"));
        resetButton->SetPressCallback([this](){
            OnResetButtonPressed();
        });

        // Search button.
        SidneyButton* searchButton = SidneyUtil::CreateSmallButton(searchBarPanel->GetOwner());
        searchButton->SetName("SearchButton");
        searchButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        searchButton->GetRectTransform()->SetAnchoredPosition(426.0f, -17.0f);
        searchButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Search"));
        searchButton->SetPressCallback([this](){
            OnSearchButtonPressed();
        });

        // Text input field.
        UINineSlice* searchInputPanel = UI::CreateWidgetActor<UINineSlice>("SearchBar", searchBarPanel, SidneyUtil::GetGrayBoxParams(Color32::Black));
        searchInputPanel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        searchInputPanel->GetRectTransform()->SetAnchoredPosition(100.0f, -17.0f);
        searchInputPanel->GetRectTransform()->SetSizeDelta(320.0f, 15.0f);

        mTextInput = UI::CreateWidgetActor<UITextInput>("TextInput", searchInputPanel);
        mTextInput->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mTextInput->GetRectTransform()->SetAnchoredPosition(4.0f, 0.0f);
        mTextInput->GetRectTransform()->SetSizeDelta(316.0f, 15.0f);
        mTextInput->SetFont(gAssetManager.LoadFont("F_TIMES.FON"));
        mTextInput->SetVerticalAlignment(VerticalAlignment::Bottom);
        mTextInput->SetText("");
        mTextInput->SetMaxLength(35); // should be enough?

        // The user isn't allowed to unfocus this text input.
        mTextInput->AllowInputToChangeFocus(false);

        // Create text input field caret.
        UIImage* caretImage = UI::CreateWidgetActor<UIImage>("Caret", mTextInput);
        caretImage->SetTexture(&Texture::White);
        caretImage->GetRectTransform()->SetAnchor(AnchorPreset::LeftStretch, false);
        caretImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        caretImage->GetRectTransform()->SetSizeDelta(2.0f, 0.0f);

        mTextInput->SetCaret(caretImage);
        mTextInput->SetCaretBlinkInterval(0.5f);
    }

    // Add navigation bar (bottom area).
    {
        // Background.
        UINineSlice* navBarPanel = UI::CreateWidgetActor<UINineSlice>("NavBox", mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        navBarPanel->GetRectTransform()->SetPivot(0.0f, 1.0f);
        navBarPanel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        navBarPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -390.0f);
        navBarPanel->GetRectTransform()->SetSizeDelta(kWebpageWidth, 28.0f);

        // Back button.
        mHistoryBackButton = SidneyUtil::CreateSmallButton(navBarPanel->GetOwner());
        mHistoryBackButton->SetName("BackButton");
        mHistoryBackButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mHistoryBackButton->GetRectTransform()->SetAnchoredPosition(63.0f, -8.0f);
        mHistoryBackButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Back"));
        mHistoryBackButton->SetPressCallback([this](){
            OnBackButtonPressed();
        });
        mHistoryBackButton->GetButton()->SetCanInteract(false);

        // Forward button.
        mHistoryForwardButton = SidneyUtil::CreateSmallButton(navBarPanel->GetOwner());
        mHistoryForwardButton->SetName("ForwardButton");
        mHistoryForwardButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mHistoryForwardButton->GetRectTransform()->SetAnchoredPosition(155.0f, -8.0f);
        mHistoryForwardButton->SetText(SidneyUtil::GetSearchLocalizer().GetText("Forward"));
        mHistoryForwardButton->SetPressCallback([this](){
            OnForwardButtonPressed();
        });
        mHistoryForwardButton->GetButton()->SetCanInteract(false);

        // Hovered link target label.
        mLinkTargetLabel = UI::CreateWidgetActor<UILabel>("HoveredLinkLabel", navBarPanel->GetOwner());
        mLinkTargetLabel->GetRectTransform()->SetAnchor(AnchorPreset::Right);
        mLinkTargetLabel->GetRectTransform()->SetAnchoredPosition(-5.0f, 0.0f);
        mLinkTargetLabel->GetRectTransform()->SetSizeDelta(0.0f, 28.0f);
        mLinkTargetLabel->SetFont(gAssetManager.LoadFont("F_TIMES.FON"));
        mLinkTargetLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mLinkTargetLabel->SetVerticalAlignment(VerticalAlignment::Center);
    }

    // Add search results web page area (middle).
    {
        UIImage* resultsPanel = UI::CreateWidgetActor<UIImage>("WebPageBackground", mRoot);
        resultsPanel->SetColor(Color32(0, 0, 0, 128));
        resultsPanel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        resultsPanel->GetRectTransform()->SetAnchoredPosition(60.0f, -136.0f);
        resultsPanel->GetRectTransform()->SetSizeDelta(kWebpageWidth, 254.0f);
        mWebPageRoot = resultsPanel->GetOwner();

        mWebPageWidgetsCanvas = UI::CreateCanvas("WebPage", resultsPanel, 1);
        mWebPageWidgetsCanvas->SetMasked(true);
        mWebPageWidgetsCanvas->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mWebPageWidgetsCanvas->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        mWebPageWidgetsCanvas->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);

        mWebPageScrollRect = new UIScrollRect(mWebPageWidgetsCanvas->GetOwner());
        mWebPageScrollRect->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mWebPageScrollRect->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
        mWebPageScrollRect->SetScrollbarWidth(kScrollbarWidth);

        // Web page area is disabled by default. It enables when you search for something.
        mWebPageRoot->SetActive(false);
    }

    // Read in all the search terms and correlated pages.
    {
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
    }

    // Read in all the dialogue triggers for visiting certain pages under certain conditions.
    {
        TextAsset* textFile = gAssetManager.LoadText("SIDNEYDIALOG.TXT");
        IniParser parser(textFile->GetText(), textFile->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);

        // Each section's name correlates to the associated HTML page.
        // Each section then has key/value pairs with trigger conditions and results.
        IniSection section;
        while(parser.ReadNextSection(section))
        {
            DialogueTrigger trigger;
            for(auto& line : section.lines)
            {
                if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "ME"))
                {
                    trigger.licensePlate = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Flag"))
                {
                    trigger.flagToSet = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Flags"))
                {
                    trigger.requiredFlags = StringUtil::Split(line.entries[0].value, ',');
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "LSR"))
                {
                    trigger.lsrMin = line.entries[0].GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "LSRMax"))
                {
                    trigger.lsrMax = line.entries[0].GetValueAsInt();
                }
            }
            mDialogueTriggers[section.name] = trigger;
        }
    }

    // Hide by default.
    Hide();
}

void SidneySearch::Show()
{
    // Show this screen.
    mRoot->SetActive(true);

    // Focus text input.
    mTextInput->Focus();

    // Handle repopulating this screen when it is shown after a save game load.
    if(!mHistory.empty() && mHistoryIndex >= 0 && mHistoryIndex < mHistory.size())
    {
        if(!StringUtil::EqualsIgnoreCase(mCurrentPageName, mHistory[mHistoryIndex]))
        {
            ShowWebPage(mHistory[mHistoryIndex]);
        }
    }

    // Make sure history menu is populated, in case we come back here after loading a save.
    RefreshHistoryMenu();
}

void SidneySearch::Hide()
{
    // Hide the screen.
    mRoot->SetActive(false);

    // Make sure the input is not focused.
    mTextInput->Unfocus();
}

void SidneySearch::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }

    // If enter is pressed, and text input is focused, act like a search was done.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN) && mTextInput->IsFocused())
    {
        OnSearchButtonPressed();
    }

    // If a link is hovered, we need to update the link target label.
    mLinkTargetLabel->SetText("");
    for(auto& entry : mWebPageLinks)
    {
        if(entry.first->IsHovered())
        {
            mLinkTargetLabel->SetText(entry.second);
            break;
        }
    }

    // Keep menu bar updated.
    mMenuBar.Update();

    // When search is active, and the scene is the top layer, the text input should stay focused.
    // On the other hand, if another layer is open (e.g. inventory, save/load, etc), we should unfocus until we return to this layer.
    bool sceneIsTopLayer = gLayerManager.IsTopLayer("SceneLayer");
    if(sceneIsTopLayer)
    {
        // Only take focus if nothing else has it - fixes stealing focus from the debug console, for example.
        if(!gInputManager.IsTextInput())
        {
            mTextInput->Focus();
        }
    }
    else
    {
        mTextInput->Unfocus();
    }
}

void SidneySearch::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mHistory));
    ps.Xfer(PERSIST_VAR(mHistoryIndex));

    std::string inputText = mTextInput->GetText();
    ps.Xfer("mTextInput", inputText);
    if(ps.IsLoading())
    {
        mTextInput->SetText(inputText);
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
    bool isTextContinuation = false;
    float lastFontGlyphHeight = 0.0f;
    bool hadBulletOnLastLine = false;
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

                /*
                // NOTE: this can be helpful to debug what element is being rendered, and what tags should be applied to it.
                std::string toRender;
                for(int i = 0; i < parseStack.size(); ++i)
                {
                    toRender += parseStack[i]->tagOrData + " > ";
                }
                printf("Render HTML %s\n", toRender.c_str());
                */

                // For text rendering, font parameters.
                int fontSize = 0;
                bool bold = false;
                bool italic = false;

                // For links, what page to link to.
                std::string link;

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
                std::string fontName = (fontSize == 2 ? "F_TIMES_2.FON" : "F_TIMES.FON");
                if(bold && italic)
                {
                    printf("HTML: unsupported text format bold AND italic...\n");
                }
                else if(bold)
                {
                    fontName = (fontSize == 2 ? "F_TIMES_B_2.FON" : "F_TIMES_B.FON");
                }
                else if(italic)
                {
                    if(fontSize != 0)
                    {
                        printf("HTML: unsupported text format (italic font size %i)\n", fontSize);
                    }
                    fontName = "F_TIMES_I.FON";
                }
                //TODO: Assets exist for underlined font (normal and +2) and bold+underlined font (normal and +2). But never used in-game?

                // Links always use the link variant of the font.
                if(!link.empty())
                {
                    fontName = "F_TIMES_L.FON";
                }

                // Handle rendering the leaf data.
                const float kLineBreakHeight = 15.0f;
                if(StringUtil::EqualsIgnoreCase(element.tagOrData, "P") ||
                   StringUtil::EqualsIgnoreCase(element.tagOrData, "BR"))
                {
                    if(isTextContinuation && !hadBulletOnLastLine)
                    {
                        resultsPos.y -= lastFontGlyphHeight;
                    }

                    // This just breaks to the next line.
                    resultsPos.x = 0.0f;
                    resultsPos.y -= kLineBreakHeight;

                    hadBulletOnLastLine = false;
                    isTextContinuation = false;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "LI"))
                {
                    // List items should always be on a new line.
                    // Usually the data does this manually with a <BR> at the end, but sometimes it's forgotten.
                    if(resultsPos.x > 0.0f)
                    {
                        resultsPos.x = 0.0f;
                        resultsPos.y -= kLineBreakHeight;
                    }

                    UIImage* bulletImage = UI::CreateWidgetActor<UIImage>("BulletPoint", mWebPageScrollRect);
                    bulletImage->SetTexture(gAssetManager.LoadTexture("SIDNEYBULLET.BMP", AssetScope::Global), true);
                    bulletImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    bulletImage->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    mWebPageWidgets.push_back(bulletImage);

                    // Move over next to the bullet, but stay on the same line.
                    // The next thing (likely some text) will go next to the bullet.
                    resultsPos.x += bulletImage->GetRectTransform()->GetRect().width + 2.0f;

                    hadBulletOnLastLine = true;
                    isTextContinuation = false;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "HR"))
                {
                    // Always flush with left side.
                    resultsPos.x = 0.0f;
                    resultsPos.y -= kLineBreakHeight;

                    // Make an image with the horizontal rule.
                    UIImage* hrImage = UI::CreateWidgetActor<UIImage>("HR", mWebPageScrollRect);
                    hrImage->SetTexture(gAssetManager.LoadTexture("HORIZONTALRULE.BMP"), true);
                    hrImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    hrImage->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    hrImage->GetRectTransform()->SetSizeDeltaX(kWebpageContentsWidth);
                    mWebPageWidgets.push_back(hrImage);

                    // Jump down below the image.
                    resultsPos.x = 0.0f;
                    resultsPos.y -= hrImage->GetRectTransform()->GetRect().height;

                    isTextContinuation = false;
                }
                else if(StringUtil::EqualsIgnoreCase(element.tagOrData, "IMG"))
                {
                    // Create the image at the appropriate size.
                    UIImage* image = UI::CreateWidgetActor<UIImage>("Image", mWebPageScrollRect);
                    image->SetTexture(gAssetManager.LoadTexture(element.attributes[0].value, AssetScope::Global), true);
                    image->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    image->GetRectTransform()->SetAnchoredPosition(resultsPos);
                    mWebPageWidgets.push_back(image);

                    // Jump down below the image.
                    resultsPos.x = 0.0f;
                    resultsPos.y -= image->GetRectTransform()->GetRect().height;

                    isTextContinuation = false;
                }
                else // text or text link
                {
                    Font* font = gAssetManager.LoadFont(fontName);
                    lastFontGlyphHeight = font->GetGlyphHeight();

                    // If we're continuing the text on an existing line, we probably want to add a bit of x-pos for the space between words.
                    // However, the exception is when the next character is punctuation.
                    const float kWordSeparation = 6.0f;
                    if(isTextContinuation)
                    {
                        if(!element.tagOrData.empty() &&
                           element.tagOrData[0] != '.' &&
                           element.tagOrData[0] != ',' &&
                           element.tagOrData[0] != ')')
                        {
                            resultsPos.x += kWordSeparation;
                        }
                    }

                    // The logic here is a bit complex due to having to *continue* pre-existing lines if they start midway through a previous line of text.
                    const float kMinWidthToContinueLine = 50.0f;
                    if(!isTextContinuation)
                    {
                        // The logic is easiest if this IS NOT a text continuation. We just create a normal label because everything is left-aligned already.
                        UILabel* label = CreateWebPageText(element.tagOrData, font, resultsPos, kWebpageContentsWidth, link);
                        resultsPos.x = label->GetNextCharPos().x;
                        resultsPos.y -= label->GetTextHeight();
                        resultsPos.y += font->GetGlyphHeight();
                    }
                    else
                    {
                        // Ok, so we are rendering MORE text, and the last thing we rendered was ALSO text.
                        // We need to make sure this new text is positioned such that it appears to flow naturally after the previous text.

                        // The easiest way to achieve this right now is to finish the last line with one label, then do the rest of the text in a new label.

                        // First, based on the space available in the last line, see if we can fit it all in there.
                        Rect r(0, 0, kWebpageContentsWidth - resultsPos.x, 1000.0f);
                        TextLayout layout(r, font, HorizontalAlignment::Left, VerticalAlignment::Top, HorizontalOverflow::Wrap, VerticalOverflow::Overflow);
                        layout.AddLine(element.tagOrData);

                        // Best case, we can fit this all in the space on the remaining line.
                        if(layout.GetLineCount() == 1)
                        {
                            UILabel* label = CreateWebPageText(element.tagOrData, font, resultsPos, kWebpageContentsWidth - resultsPos.x, link);
                            resultsPos.x += label->GetNextCharPos().x;
                            resultsPos.y -= label->GetTextHeight();
                            resultsPos.y += font->GetGlyphHeight();

                            // For proper link highlight and click areas, resize the label to equal the text width.
                            if(!link.empty())
                            {
                                label->GetRectTransform()->SetSizeDeltaX(label->GetTextWidth());
                            }
                        }
                        else
                        {
                            // This is the harder case, where the text doesn't fit in the remaining space on the current line.
                            // First, we need to split the text based on what fits on the current line and what fits on the next line onwards.
                            int lastCharIndexOnFirstLine = 0;
                            float firstY = layout.GetChar(0)->pos.y;
                            for(auto& c : layout.GetChars())
                            {
                                if(c.pos.y == firstY)
                                {
                                    ++lastCharIndexOnFirstLine;
                                }
                                else
                                {
                                    break;
                                }
                            }

                            std::string firstLineText = element.tagOrData.substr(0, lastCharIndexOnFirstLine);
                            std::string remainingText = element.tagOrData.substr(lastCharIndexOnFirstLine + 1);

                            // Create one label for the text that'll fit in the remaining space on the current line.
                            UILabel* lineOne = CreateWebPageText(firstLineText, font, resultsPos, kWebpageContentsWidth - resultsPos.x, link);
                            resultsPos.x = 0.0f;
                            resultsPos.y -= lineOne->GetTextHeight();

                            // Create another label for any remaining text that must be shunted to the next line.
                            UILabel* remaining = CreateWebPageText(remainingText, font, resultsPos, kWebpageContentsWidth, link);
                            resultsPos.x = remaining->GetNextCharPos().x;
                            resultsPos.y -= remaining->GetTextHeight();
                            resultsPos.y += font->GetGlyphHeight();

                            // If this is a link, resize the with of the labels to ensure click/highlight area is correct.
                            if(!link.empty())
                            {
                                lineOne->GetRectTransform()->SetSizeDeltaX(lineOne->GetTextWidth());
                                remaining->GetRectTransform()->SetSizeDeltaX(remaining->GetTextWidth());
                            }
                        }
                    }

                    // If there's enough space remaining at the end of the current line, we could continue the next piece of text on this line.
                    isTextContinuation = kWebpageContentsWidth - resultsPos.x > kMinWidthToContinueLine;

                    // But if we won't continue text on this line, do a carriage return to the next line.
                    if(!isTextContinuation)
                    {
                        resultsPos.x = 0.0f;
                        resultsPos.y -= font->GetGlyphHeight();
                    }
                }
            }
        }
    }

    // Create some extra space at the bottom of the web page using an empty label.
    UILabel* spacerLabel = UI::CreateWidgetActor<UILabel>("Spacer", mWebPageScrollRect);
    spacerLabel->GetRectTransform()->SetSizeDeltaY(0.0f);
    spacerLabel->GetRectTransform()->SetAnchoredPosition(resultsPos);
    mWebPageWidgets.push_back(spacerLabel);

    // Make sure the web page root UI element is active, so you can see everything!
    mWebPageRoot->SetActive(true);

    // Save current page name.
    mCurrentPageName = pageName;

    // We are now on this web page. Trigger any known events that can happen when entering this page.
    TriggerWebPageEvents(pageName);
}

UILabel* SidneySearch::CreateWebPageText(const std::string& text, Font* font, const Vector2& pos, float width, std::string& link)
{
    // Create a label with the appropriate font and text.
    UILabel* label = UI::CreateWidgetActor<UILabel>("PageText", mWebPageScrollRect);
    label->SetFont(font);
    label->SetText(text);

    // Alignment and overflow settings are always the same.
    label->SetHorizonalAlignment(HorizontalAlignment::Left);
    label->SetVerticalAlignment(VerticalAlignment::Top);
    label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
    label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    // Set position and width.
    label->GetRectTransform()->SetAnchoredPosition(pos);
    label->GetRectTransform()->SetSizeDeltaX(width);

    // Setting the size to the text height ensures a tight fit vertically.
    label->GetRectTransform()->SetSizeDeltaY(label->GetTextHeight());

    // Add to widgets.
    mWebPageWidgets.push_back(label);

    // If this is a link, also make this bit of text act as a button.
    if(!link.empty())
    {
        UIButton* button = label->GetOwner()->AddComponent<UIButton>();
        button->SetHighlightCursorOnHover(true);
        button->SetPressCallback([link, this](UIButton* button){
            ShowWebPage(link);

            // When you click on a link, it gets added to the web page history.
            AddToHistory(link);
            RefreshHistoryMenu();
        });
        mWebPageWidgets.push_back(button);
        mWebPageLinks[button] = link;
    }
    return label;
}

void SidneySearch::TriggerWebPageEvents(const std::string& pageName)
{
    // When you visit a page, we *may* need to trigger some dialogue.
    auto dialogueIt = mDialogueTriggers.find(pageName);
    if(dialogueIt != mDialogueTriggers.end())
    {
        // There is dialogue to trigger, now let's see if we meet all the conditions...

        // First, these triggers only occur one time.
        // If the flag to set upon triggering this dialogue is already set, then we already triggered it - don't do it again.
        bool metAllRequirements = !gGameProgress.GetFlag(dialogueIt->second.flagToSet);

        // Second, you must be within the required LSR steps min/max inclusive.
        if(metAllRequirements)
        {
            int currentLsrStep = SidneyUtil::GetCurrentLSRStep();
            metAllRequirements = (dialogueIt->second.lsrMin < 0 || currentLsrStep >= dialogueIt->second.lsrMin) &&
                                 (dialogueIt->second.lsrMax < 0 || currentLsrStep <= dialogueIt->second.lsrMax);
        }

        // Finally, any flag conditions must be met.
        if(metAllRequirements)
        {
            for(auto& flag : dialogueIt->second.requiredFlags)
            {
                // Figure out if this requirement was met. The ability to put "!SomeFlag" makes it slightly more complex.
                bool metFlagRequirement = false;
                if(flag.front() == '!')
                {
                    metFlagRequirement = !gGameProgress.GetFlag(flag.substr(1));
                }
                else
                {
                    metFlagRequirement = gGameProgress.GetFlag(flag);
                }

                // If any flag requirement isn't met, then we haven't met all the requirements.
                if(!metFlagRequirement)
                {
                    metAllRequirements = false;
                    break;
                }
            }
        }

        // Finally, we can play the dialogue trigger if we still met all requirements.
        if(metAllRequirements)
        {
            // Play the dialogue.
            std::string command = StringUtil::Format("wait StartDialogue(\"%s\", 1)", dialogueIt->second.licensePlate.c_str());
            gActionManager.ExecuteSheepAction(command, [](const Action* action){

                // One of the conditions for ending Day 3, 7AM is to search for the temple floor plan. This causes some dialogue to play.
                // SO, if this is the last condition met, after that dialogue plays, we should force end the timeblock by exiting Sidney.
                if(gGameProgress.GetTimeblock() == Timeblock(3, 7))
                {
                    SidneyUtil::CheckForceExitSidney307A();
                }
            });

            // Set the flag.
            gGameProgress.SetFlag(dialogueIt->second.flagToSet);
        }
    }

    // Visiting certain pages may trigger score gain.
    auto scoreEventIt = mWebPageScoreEvents.find(pageName);
    if(scoreEventIt != mWebPageScoreEvents.end())
    {
        gGameProgress.ChangeScore(scoreEventIt->second);
    }

    // Visiting certain pages may trigger a flag being set for internal logic.
    auto flagEventIt = mWebPageFlagEvents.find(pageName);
    if(flagEventIt != mWebPageFlagEvents.end())
    {
        gGameProgress.SetFlag(flagEventIt->second);
    }
}

void SidneySearch::AddToHistory(const std::string& pageName)
{
    // If we add a new page to history, but we aren't at the newest page in the history, anything newer gets truncated.
    if(mHistoryIndex != mHistory.size() - 1)
    {
        mHistory.erase(mHistory.begin() + mHistoryIndex + 1, mHistory.end());
    }

    // Add to history, keeping max size in mind.
    if(mHistory.size() == kMaxHistorySize)
    {
        mHistory.erase(mHistory.begin());
    }
    mHistory.push_back(pageName);

    // Our current history page is now this one.
    mHistoryIndex = mHistory.size() - 1;
}

void SidneySearch::RefreshHistoryMenu()
{
    // Clear existing dropdown choices.
    mMenuBar.ClearDropdownChoices(0);

    // Repopulate dropdown choices with page history, starting with latest at top.
    for(int i = mHistory.size() - 1; i >= 0; --i)
    {
        std::string dropdownText = StringUtil::Format("%i. %s", i + 1, StringUtil::ToLowerCopy(mHistory[i]).c_str());
        std::string link = mHistory[i];
        mMenuBar.AddDropdownChoice(dropdownText, [link, this](){
            ShowWebPage(link);
        });
    }

    // If the history menu changes (meaning history has been added or removed), it may also affect the history buttons.
    RefreshHistoryButtons();
}

void SidneySearch::RefreshHistoryButtons()
{
    // Update whether back/forward buttons are enabled.
    // They aren't enabled if there's no history...
    if(mHistoryIndex == -1 || mHistory.empty())
    {
        mHistoryBackButton->GetButton()->SetCanInteract(false);
        mHistoryForwardButton->GetButton()->SetCanInteract(false);
    }
    else
    {
        // If there is history, enable them when there is history in that direction.
        mHistoryForwardButton->GetButton()->SetCanInteract(mHistoryIndex < mHistory.size() - 1);
        mHistoryBackButton->GetButton()->SetCanInteract(mHistoryIndex > 0);
    }
}

void SidneySearch::ClearWebPage()
{
    // Reset web page scroll back to the top.
    mWebPageScrollRect->SetNormalizedScrollValue(0.0f);

    // Destroy all widgets that were created for this web page.
    for(UIWidget* widget : mWebPageWidgets)
    {
        widget->GetOwner()->Destroy();

        // Due to a bug/complexity (widget destructors can't safely remove themselves from their parent Canvases) - we must manually remove from canvas here.
        //TODO: Would be great for Widgets to be able to handle this internally...
        mWebPageWidgetsCanvas->RemoveWidget(widget);
    }
    mWebPageWidgets.clear();

    // Clear list of web page links (they were destroyed in the previous for loop).
    mWebPageLinks.clear();

    // Hide the web page root entirely.
    mWebPageRoot->SetActive(false);
}

void SidneySearch::OnSearchButtonPressed()
{
    auto it = mSearchTerms.find(mTextInput->GetText());
    if(it != mSearchTerms.end())
    {
        ShowWebPage(it->second);

        // When you search for a link, it gets added to the web page history.
        AddToHistory(it->second);
        RefreshHistoryMenu();
    }
    else
    {
        // In the original game, they keep the text input focused when the popup appears.
        // But also still stop input from going to the text input.
        // In our case, for simplicity, I think I'm OK temporarily unfocusing...
        mTextInput->Unfocus();

        // Show a popup that says "what you're searching for can't be found."
        mPopup->ResetToDefaults();
        mPopup->SetTextAlignment(HorizontalAlignment::Center);
        mPopup->SetText(SidneyUtil::GetSearchLocalizer().GetText("NotFound"));
        mPopup->ShowOneButton([this](){
            mTextInput->Focus();
        });
    }
}

void SidneySearch::OnResetButtonPressed()
{
    // Reset clears any text input and hides the current web page.
    mTextInput->Clear();
    ClearWebPage();
}

void SidneySearch::OnBackButtonPressed()
{
    mHistoryIndex = Math::Max(mHistoryIndex - 1, 0);
    ShowWebPage(mHistory[mHistoryIndex]);
    RefreshHistoryButtons();
}

void SidneySearch::OnForwardButtonPressed()
{
    mHistoryIndex = Math::Min<int>(mHistoryIndex + 1, mHistory.size() - 1);
    ShowWebPage(mHistory[mHistoryIndex]);
    RefreshHistoryButtons();
}
