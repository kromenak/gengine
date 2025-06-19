#include "HelpScreen.h"

#include "AssetManager.h"
#include "Font.h"
#include "IniParser.h"
#include "TextAsset.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

HelpScreen::HelpScreen() : Actor("HelpScreen", TransformType::RectTransform),
    mLayer("HelpLayer")
{
    // When the help screen shows, all underlying audio stops entirely.
    mLayer.OverrideAudioState(true);

    // Add a fullscreen canvas that tints whatever's under it.
    UI::AddCanvas(this, 39, Color32(0, 0, 0, 128));

    Font* titleFont = nullptr;
    UINineSliceParams boxParams;
    Texture* nextButtonUpTexture = nullptr;
    Texture* nextButtonDownTexture = nullptr;
    Texture* nextButtonHoverTexture = nullptr;
    Texture* prevButtonUpTexture = nullptr;
    Texture* prevButtonDownTexture = nullptr;
    Texture* prevButtonHoverTexture = nullptr;
    Texture* exitButtonUpTexture = nullptr;
    Texture* exitButtonDownTexture = nullptr;
    Texture* exitButtonHoverTexture = nullptr;
    {
        TextAsset* textFile = gAssetManager.LoadText("HELPSCREEN.TXT", AssetScope::Manual);
        IniParser parser(textFile->GetText(), textFile->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);
        parser.ParseAll();

        int pageCount = 0;
        IniSection rootSection = parser.GetSection("");
        for(auto& line : rootSection.lines)
        {
            IniKeyValue& entry = line.entries[0];
            if(entry.key[0] == ';') { continue; }

            if(StringUtil::EqualsIgnoreCase(entry.key, "Alpha"))
            {
                boxParams.centerColor.a = static_cast<unsigned char>(entry.GetValueAsFloat() * 255);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "BackColor"))
            {
                Color32 backColor = entry.GetValueAsColor32();
                boxParams.centerColor.r = backColor.r;
                boxParams.centerColor.g = backColor.g;
                boxParams.centerColor.b = backColor.b;
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "TitleFont"))
            {
                titleFont = gAssetManager.LoadFont(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "KeyFont"))
            {
                mKeyFont = gAssetManager.LoadFont(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "UseFont"))
            {
                mUseFont = gAssetManager.LoadFont(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "BLCornerSprite"))
            {
                boxParams.bottomLeftTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "TLCornerSprite"))
            {
                boxParams.topLeftTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "BRCornerSprite"))
            {
                boxParams.bottomRightTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "TRCornerSprite"))
            {
                boxParams.topRightTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "SideSprite"))
            {
                boxParams.leftTexture = boxParams.rightTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "TopSprite"))
            {
                boxParams.topTexture = boxParams.bottomTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "NumPages"))
            {
                pageCount = entry.GetValueAsInt();
                for(int i = 0; i < pageCount; ++i)
                {
                    mPages.emplace_back();
                }
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "NextBtnUp"))
            {
                nextButtonUpTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "NextBtnDown"))
            {
                nextButtonDownTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "NextBtnHov"))
            {
                nextButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "PrevBtnUp"))
            {
                prevButtonUpTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "PrevBtnDown"))
            {
                prevButtonDownTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "PrevBtnHov"))
            {
                prevButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "ExitBtnUp"))
            {
                exitButtonUpTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "ExitBtnDown"))
            {
                exitButtonDownTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::EqualsIgnoreCase(entry.key, "ExitBtnHov"))
            {
                exitButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "Size"))
            {
                int pageIndex = (entry.key.back() - '1');
                mPages[pageIndex].size = entry.GetValueAsVector2();
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "Title"))
            {
                int pageIndex = (entry.key.back() - '1');
                mPages[pageIndex].title = entry.value;
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "NextBtnPos"))
            {
                int pageIndex = (entry.key.back() - '1');
                mPages[pageIndex].nextButtonPosition = entry.GetValueAsVector2();
                mPages[pageIndex].nextButtonPosition.y *= -1.0f;
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "PrevBtnPos"))
            {
                int pageIndex = (entry.key.back() - '1');
                mPages[pageIndex].prevButtonPosition = entry.GetValueAsVector2();
                mPages[pageIndex].prevButtonPosition.y *= -1.0f;
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "ExitBtnPos"))
            {
                int pageIndex = (entry.key.back() - '1');
                mPages[pageIndex].exitButtonPosition = entry.GetValueAsVector2();
                mPages[pageIndex].exitButtonPosition.y *= -1.0f;
            }
            else if(StringUtil::StartsWithIgnoreCase(entry.key, "PageSections"))
            {
                int pageIndex = (entry.key.back() - '1');
                std::vector<std::string> sectionNames = StringUtil::Split(entry.value, ',', true);
                for(auto& sectionName : sectionNames)
                {
                    StringUtil::Trim(sectionName);
                    mPages[pageIndex].sections[sectionName] = Section();
                }
            }
        }

        // For each section in each page, parse the corresponding section in the INI file.
        for(auto& page : mPages)
        {
            for(auto& sectionEntry : page.sections)
            {
                IniSection iniSection = parser.GetSection(sectionEntry.first);
                for(auto& line : iniSection.lines)
                {
                    IniKeyValue& entry = line.entries[0];
                    if(entry.key[0] == ';') { continue; }

                    if(StringUtil::EqualsIgnoreCase(entry.key, "Pos"))
                    {
                        sectionEntry.second.position = entry.GetValueAsVector2();
                        sectionEntry.second.position.y *= -1.0f;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "Width"))
                    {
                        sectionEntry.second.width = entry.GetValueAsFloat();
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "KeyCode"))
                    {
                        sectionEntry.second.actionId = entry.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "UseText"))
                    {
                        sectionEntry.second.useText = entry.value;
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "Sprite"))
                    {
                        sectionEntry.second.texture = gAssetManager.LoadTexture(entry.value);
                    }
                }
            }
        }

        delete textFile;
    }

    // Create background box.
    UINineSlice* box = UI::CreateWidgetActor<UINineSlice>("Box", this, boxParams);
    box->GetRectTransform()->SetSizeDelta(600.0f, 245.0f);
    mBox = box;

    // Create title text.
    UILabel* titleLabel = UI::CreateWidgetActor<UILabel>("Title", box);
    titleLabel->SetFont(titleFont);
    titleLabel->SetText("Title");
    titleLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    titleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
    titleLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -10.0f);
    titleLabel->FitRectTransformToText();
    mTitleLabel = titleLabel;

    mPrevButton = UI::CreateWidgetActor<UIButton>("PrevButton", box);
    mPrevButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mPrevButton->SetUpTexture(prevButtonUpTexture);
    mPrevButton->SetDownTexture(prevButtonDownTexture);
    mPrevButton->SetHoverTexture(prevButtonHoverTexture);
    mPrevButton->SetPressCallback([this](UIButton* button){
        PreviousPage();
    });
    mPrevButton->SetTooltipText("prevgamehelp");

    mNextButton = UI::CreateWidgetActor<UIButton>("NextButton", box);
    mNextButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mNextButton->SetUpTexture(nextButtonUpTexture);
    mNextButton->SetDownTexture(nextButtonDownTexture);
    mNextButton->SetHoverTexture(nextButtonHoverTexture);
    mNextButton->SetPressCallback([this](UIButton* button){
        NextPage();
    });
    mNextButton->SetTooltipText("nextgamehelp");

    mExitButton = UI::CreateWidgetActor<UIButton>("ExitButton", box);
    mExitButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    mExitButton->SetUpTexture(exitButtonUpTexture);
    mExitButton->SetDownTexture(exitButtonDownTexture);
    mExitButton->SetHoverTexture(exitButtonHoverTexture);
    mExitButton->SetPressCallback([this](UIButton* button){
        Hide();
    });
    mExitButton->SetTooltipText("exitgamehelp");

    // Hide by default.
    SetActive(false);
}

void HelpScreen::Show()
{
    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);

    // Start on first page.
    SetPage(0);
}

void HelpScreen::Hide()
{
    // Pop layer off stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);
}

void HelpScreen::SetPage(int pageIndex)
{
    // Update page index.
    mPageIndex = pageIndex;

    // Set box size.
    mBox->GetRectTransform()->SetSizeDelta(mPages[pageIndex].size);

    // Show/hide next/prev buttons.
    mPrevButton->SetEnabled(pageIndex > 0);
    mNextButton->SetEnabled(pageIndex < mPages.size() - 1);

    mPrevButton->GetRectTransform()->SetAnchoredPosition(mPages[pageIndex].prevButtonPosition);
    mNextButton->GetRectTransform()->SetAnchoredPosition(mPages[pageIndex].nextButtonPosition);
    mExitButton->GetRectTransform()->SetAnchoredPosition(mPages[pageIndex].exitButtonPosition);

    // Set title text.
    mTitleLabel->SetText(mPages[pageIndex].title);

    // Hide all previously created labels/images.
    for(auto& label : mKeyLabels)
    {
        label->SetEnabled(false);
    }
    for(auto& label : mUseLabels)
    {
        label->SetEnabled(false);
    }
    for(auto& image : mImages)
    {
        image->SetEnabled(false);
    }

    // Populate all sections for this page.
    // Labels and images are recycled across pages.
    int keyLabelCounter = 0;
    int useLabelCounter = 0;
    int imageCounter = 0;
    for(auto& entry : mPages[pageIndex].sections)
    {
        if(!entry.second.actionId.empty())
        {
            UILabel* label = nullptr;
            if(keyLabelCounter < mKeyLabels.size())
            {
                label = mKeyLabels[keyLabelCounter];
            }
            else
            {
                label = UI::CreateWidgetActor<UILabel>("KeyLabel", mBox);
                label->SetFont(mKeyFont);
                label->SetHorizonalAlignment(HorizontalAlignment::Left);
                label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
                label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                mKeyLabels.push_back(label);
            }

            label->SetEnabled(true);
            label->GetRectTransform()->SetAnchoredPosition(entry.second.position);
            label->GetRectTransform()->SetSizeDelta(entry.second.width, mKeyFont->GetGlyphHeight());
            label->SetText(ActionIdToKey(entry.second.actionId));
            ++keyLabelCounter;
        }
        else if(!entry.second.useText.empty())
        {
            UILabel* label = nullptr;
            if(useLabelCounter < mUseLabels.size())
            {
                label = mUseLabels[useLabelCounter];
            }
            else
            {
                label = UI::CreateWidgetActor<UILabel>("UseLabel", mBox);
                label->SetFont(mUseFont);
                label->SetHorizonalAlignment(HorizontalAlignment::Left);
                label->SetVerticalAlignment(VerticalAlignment::Top);
                label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
                label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                mUseLabels.push_back(label);
            }

            label->SetEnabled(true);
            label->GetRectTransform()->SetAnchoredPosition(entry.second.position);
            label->GetRectTransform()->SetSizeDelta(entry.second.width, mUseFont->GetGlyphHeight());
            label->SetText(entry.second.useText);
            ++useLabelCounter;
        }
        else if(entry.second.texture != nullptr)
        {
            UIImage* image = nullptr;
            if(imageCounter < mImages.size())
            {
                image = mImages[imageCounter];
            }
            else
            {
                image = UI::CreateWidgetActor<UIImage>("UseLabel", mBox);
                image->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                mImages.push_back(image);
            }

            image->SetEnabled(true);
            image->GetRectTransform()->SetAnchoredPosition(entry.second.position);
            image->SetTexture(entry.second.texture, true);
            ++imageCounter;
        }
    }
}

void HelpScreen::PreviousPage()
{
    // Go to previous page if there is one.
    if(mPageIndex > 0)
    {
        SetPage(mPageIndex - 1);
    }
}

void HelpScreen::NextPage()
{
    // Go to next page if there is on.
    if(mPageIndex < mPages.size() - 1)
    {
        SetPage(mPageIndex + 1);
    }
}

std::string HelpScreen::ActionIdToKey(const std::string& actionId)
{
    //TODO: Actually maintain a mapping of actions to bound keys.
    if(StringUtil::EndsWithIgnoreCase(actionId, "Help"))
    {
        return "F1";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "Help2"))
    {
        return "H";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "Inventory"))
    {
        return "I";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "Pause"))
    {
        return "P";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "Pause2"))
    {
        return "Pause";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "QuickSave"))
    {
        return "F5";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "QuickLoad"))
    {
        return "F6";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "Back"))
    {
        return "Escape";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "ForwardCamera"))
    {
        return "Up";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "ForwardCamera2"))
    {
        return "W";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "BackwardCamera"))
    {
        return "Down";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "ForwardCamera2"))
    {
        return "S";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "LeftCamera"))
    {
        return "Left";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "LeftCamera2"))
    {
        return "A";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "RightCamera"))
    {
        return "Right";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "RightCamera2"))
    {
        return "D";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "LevelCamera"))
    {
        return "Space";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "PitchCameraMod"))
    {
        return "Shift";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "LevelCameraMod"))
    {
        return "Control";
    }
    else if(StringUtil::EndsWithIgnoreCase(actionId, "FastCameraMod"))
    {
        return "Alt";
    }
    return "";
}
