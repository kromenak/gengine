#include "SidneyAnalyze.h"

#include "Actor.h"
#include "AssetManager.h"
#include "RectTransform.h"
#include "SidneyFiles.h"
#include "UIImage.h"

void SidneyAnalyze::AnalyzeImage_Init()
{
    // Create a parent that contains all the image analysis stuff.
    mAnalyzeImageWindow = new Actor(TransformType::RectTransform);
    mAnalyzeImageWindow->GetTransform()->SetParent(mRoot->GetTransform());
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchor(AnchorPreset::CenterStretch);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchoredPosition(0.0f, 0.0f);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetSizeDelta(0.0f, 0.0f);

    // Create image that is being analyzed.
    {
        Actor* imageActor = new Actor(TransformType::RectTransform);
        imageActor->GetTransform()->SetParent(mAnalyzeImageWindow->GetTransform());

        mAnalyzeImage = imageActor->AddComponent<UIImage>();
        mAnalyzeImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
    }

    // Hide by default.
    mAnalyzeImageWindow->SetActive(false);
}

void SidneyAnalyze::AnalyzeImage_EnterState()
{
    // Show the image view.
    mAnalyzeImageWindow->SetActive(true);

    // "Text" and "Graphic" dropdowns are available when analyzing an image. Map is not.
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, false);

    // Show correct image and menu items based on current file.
    if(mAnalyzeFile->index == 20) // Parchment 1
    {
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("PARCHMENT1_BASE.BMP"), true);

        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, 0, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, 1, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, 2, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, 3, false);

        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_RotateShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ZoomClarifyIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_UseShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_EraseShapeIdx, false);
    }
    else if(mAnalyzeFile->index == 21) // Parchment 2
    {
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("PARCHMENT2_BASE.BMP"), true);

        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, true);

        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_RotateShapeIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ZoomClarifyIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_UseShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_EraseShapeIdx, false);
    }
}

void SidneyAnalyze::AnalyzeImage_OnAnalyzeButtonPressed()
{
    // Show correct analysis message depending on the file type.
    if(mAnalyzeFile->index == 20)
    {
        ShowAnalyzeMessage("AnalyzeParch1");
    }
    else if(mAnalyzeFile->index == 21)
    {
        ShowAnalyzeMessage("AnalyzeParch2");
    }
}