#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "SidneyAnagramParser.h"
#include "SidneyFiles.h"
#include "SidneyTranslate.h"
#include "SidneyUtil.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

void SidneyAnalyze::AnalyzeText_Init()
{
    // Create window for analyzing text.
    {
        UINineSlice* border = UI::CreateWidgetActor<UINineSlice>("AnalyzeTextWindow", mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(300.0f, 104.0f);
        mAnalyzeTextWindow = border->GetOwner();

        // Add divider line for box header.
        UIImage* boxHeaderDividerImage = UI::CreateWidgetActor<UIImage>("Divider", mAnalyzeTextWindow);
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        mAnalyzeTextFileNameLabel = UI::CreateWidgetActor<UILabel>("FileNameLabel", mAnalyzeTextWindow);
        mAnalyzeTextFileNameLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextFileNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mAnalyzeTextFileNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mAnalyzeTextFileNameLabel->SetMasked(true);

        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

        // Add text label.
        mAnalyzeTextLabel = UI::CreateWidgetActor<UILabel>("TextLabel", mAnalyzeTextWindow);
        mAnalyzeTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAnalyzeTextLabel->SetVerticalAlignment(VerticalAlignment::Center);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        mAnalyzeTextLabel->GetRectTransform()->SetSizeDeltaY(88.0f);
    }

    // Create window for anagram parser.
    mAnagramParser = new SidneyAnagramParser(mRoot);
}

void SidneyAnalyze::AnalyzeText_EnterState()
{
    // Show text analyze window.
    mAnalyzeTextWindow->SetActive(true);

    // Populate the file name label.
    mAnalyzeTextFileNameLabel->SetText(mSidneyFiles->GetFile(mAnalyzeFileId)->GetDisplayName());

    // "Text" dropdown is available when analyzing text - as you might expect.
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, false);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, false);

    // Populate the text field, based on file.
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText)
    {
        // Set text label. If figured out the "SUM" word to add, the text is slightly different.
        if(gGameProgress.GetFlag("ArcadiaComplete"))
        {
            mAnalyzeTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText"));
        }
        else
        {
            mAnalyzeTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText2"));
        }

        // Populate enabled menu items.
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, false);
    }
}

void SidneyAnalyze::AnalyzeText_ExitState()
{
    // Make sure the anagram parser is hidden when exiting to a different state.
    mAnagramParser->Hide();
}

void SidneyAnalyze::AnalyzeText_OnAnalyzeButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText)
    {
        ShowAnalyzeMessage("ArcadiaAnalysis", Vector2(), HorizontalAlignment::Left);
    }
}

void SidneyAnalyze::AnalyzeText_OnTranslateButtonPressed()
{
    Hide();
    mSidneyTranslate->Show(mAnalyzeFileId);
}

void SidneyAnalyze::AnalyzeText_OnAnagramParserPressed()
{
    // The anagram parser is used only in one instance, with Arcadia text while in the Orphiuchus step of LSR.
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText && SidneyUtil::GetCurrentLSRStep() >= 10)
    {
        // Hide the analyze text window.
        mAnalyzeTextWindow->SetActive(false);

        // Figure out which text will be fed into the anagram parser.
        // Text differs based on whether you've figured out the missing "SUM" word.
        std::string anagramText = gGameProgress.GetFlag("ArcadiaComplete") ?
            SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText") :
            SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText2");

        // Show the parser with the specified anagram text.
        mAnagramParser->Show(anagramText, [this](){
            mAnalyzeTextWindow->SetActive(true);
        });

        // Not 100% sure if this flag is important for anything, but it is set at this point.
        gGameProgress.SetFlag("StartArcadiaAnagram");
    }
    else
    {
        // In this case, Grace says "I don't think I need to use that on this text" - or something along those lines.
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2Z951\", 1)");
    }
}