#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "SidneyFiles.h"
#include "SidneyTranslate.h"
#include "SidneyUtil.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UIUtil.h"

void SidneyAnalyze::AnalyzeText_Init()
{
    // Create box/window for main ID making area.
    mAnalyzeTextWindow = new Actor(TransformType::RectTransform);
    mAnalyzeTextWindow->GetTransform()->SetParent(mRoot->GetTransform());
    {
        UINineSlice* border = mAnalyzeTextWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(300.0f, 104.0f);

        // Add divider line for box header.
        UIImage* boxHeaderDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(mAnalyzeTextWindow);
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        mAnalyzeTextFileNameLabel = UIUtil::NewUIActorWithWidget<UILabel>(mAnalyzeTextWindow);
        mAnalyzeTextFileNameLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextFileNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mAnalyzeTextFileNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mAnalyzeTextFileNameLabel->SetMasked(true);

        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

        // Add text label.
        mAnalyzeTextLabel = UIUtil::NewUIActorWithWidget<UILabel>(mAnalyzeTextWindow);
        mAnalyzeTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAnalyzeTextLabel->SetVerticalAlignment(VerticalAlignment::Center);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        mAnalyzeTextLabel->GetRectTransform()->SetSizeDeltaY(88.0f);
    }
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
        // Set text label.
        mAnalyzeTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText2"));

        // Populate enabled menu items.
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, false);
    }
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
    // Action depends on which file is being analyzed.
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText)
    {
        // In this case, Grace says "I don't think I need to use that on this text" - or something along those lines.
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2Z951\", 1)");
    }
}