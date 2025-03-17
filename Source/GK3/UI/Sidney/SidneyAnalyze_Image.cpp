#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "RectTransform.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
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
    // "Text" and "Graphic" dropdowns are available when analyzing an image. Map is not.
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, false);

    // Show correct image and menu items based on current file.
    if(mAnalyzeFileId == SidneyFileIds::kParchment1) // Parchment 1
    {
        mAnalyzeImageWindow->SetActive(true);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
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
    else if(mAnalyzeFileId == SidneyFileIds::kParchment2) // Parchment 2
    {
        mAnalyzeImageWindow->SetActive(true);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
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
    else if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        mAnalyzeImageWindow->SetActive(true);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -107.0f); // this one is vertically centered for some reason
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("POUSSIN.BMP"), true);

        mMenuBar.SetDropdownEnabled(kTextDropdownIdx, false);
        
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_RotateShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ZoomClarifyIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_UseShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_EraseShapeIdx, false);
    }
    else if(mAnalyzeFileId == SidneyFileIds::kTeniersPostcard2)
    {
        mAnalyzeImageWindow->SetActive(true);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("TENIERS.BMP"), true);

        mMenuBar.SetDropdownEnabled(kTextDropdownIdx, false);

        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_RotateShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ZoomClarifyIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_UseShapeIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_EraseShapeIdx, false);
    }
    else if(mAnalyzeFileId == SidneyFileIds::kSumNote)
    {
        // Even though you can analyze this file, the UI continues to show the pre-analyze UI.
        ShowPreAnalyzeUI();

        // Update menu bars to only allow the "Translate" option.
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, false);

        mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, false);
    }
}

void SidneyAnalyze::AnalyzeImage_OnAnalyzeButtonPressed()
{
    // Show correct analysis message depending on the file type.
    if(mAnalyzeFileId == SidneyFileIds::kParchment1)
    {
        ShowAnalyzeMessage("AnalyzeParch1");
    }
    else if(mAnalyzeFileId == SidneyFileIds::kParchment2)
    {
        ShowAnalyzeMessage("AnalyzeParch2");
    }
    else if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        ShowAnalyzeMessage("AnalyzePous");
    }
    else if(mAnalyzeFileId == SidneyFileIds::kTeniersPostcard1)
    {
        ShowAnalyzeMessage("AnalyzeTemp", Vector2(), HorizontalAlignment::Center);

        // There isn't actually anything interesting about this postcard (the message says "nothing interesting found").
        // Just force back to pre-analyze state in this case.
        SetState(SidneyAnalyze::State::PreAnalyze);
    }
    else if(mAnalyzeFileId == SidneyFileIds::kTeniersPostcard2)
    {
        ShowAnalyzeMessage("AnalyzePous");

        // Grace plays a bit of dialogue the first time this one gets analyzed.
        SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
        if(file != nullptr && !file->hasBeenAnalyzed)
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02o3h2zq32\", 1)");
        }
    }
    else if(mAnalyzeFileId == SidneyFileIds::kSerresHermeticSymbols)
    {
        ShowAnalyzeMessage("AnalyzeHermNote", Vector2(), HorizontalAlignment::Center);

        // You get some points for this one.
        gGameProgress.ChangeScore("e_sidney_analysis_symbols_from_serres");

        // The message says that the symbols don't exist in SIDNEY, and we need to search the internet.
        // We'll get an email later with info about these symbols.
        // But for now, there's no further analysis to be done.
        SetState(SidneyAnalyze::State::PreAnalyze);
    }
    else if(mAnalyzeFileId == SidneyFileIds::kSumNote)
    {
        // Shows a message that the text can be translated. But that's it.
        ShowAnalyzeMessage("AnalyzeSUM", Vector2(), HorizontalAlignment::Center);
    }
    else
    {
        // Unknown file - do nothing!
        SetState(SidneyAnalyze::State::PreAnalyze);
    }
}

void SidneyAnalyze::AnalyzeImage_OnExtractAnomoliesPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kParchment1)
    {
        mSecondaryAnalyzePopup->ResetToDefaults();
        mSecondaryAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ExtractParch1"));
        mSecondaryAnalyzePopup->ShowThreeButton([this](int buttonIndex){
            if(buttonIndex == 0)
            {
                mSecondaryAnalyzePopup->Hide();
                ShowAnalyzeMessage("Parch1French");
                gGameProgress.ChangeScore("e_sidney_analysis_anomalies_parch1");

                if(!gGameProgress.GetFlag("PlayedParch1FrenchMsg"))
                {
                    gActionManager.ExecuteDialogueAction("02OCR2ZMX3");
                    gGameProgress.SetFlag("PlayedParch1FrenchMsg");
                }
            }
            else if(buttonIndex == 1)
            {
                ShowAnalyzeMessage("ParchEnglish", Vector2(), HorizontalAlignment::Center);
            }
            else if(buttonIndex == 2)
            {
                ShowAnalyzeMessage("ParchLatin", Vector2(), HorizontalAlignment::Center);
            }
        });
    }
}

void SidneyAnalyze::AnalyzeImage_OnViewGeometryButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kParchment1)
    {
        printf("Added triangle\n");
        mSidneyFiles->AddFile(SidneyFileIds::kTriangleShape); // Triangle
    }
    if(mAnalyzeFileId == SidneyFileIds::kParchment2)
    {
        printf("Added circle and square\n");
        mSidneyFiles->AddFile(SidneyFileIds::kCircleShape); // Circle
        mSidneyFiles->AddFile(SidneyFileIds::kSquareShape); // Square
    }
    if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        //TODO: Play hexagram animation.

        printf("Added hexagon\n");
        mSidneyFiles->AddFile(SidneyFileIds::kHexagramShape); // Hexagram

        ShowAnalyzeMessage("GeometryPous", Vector2(180.0f, 0.0f), HorizontalAlignment::Center);
    }
}

void SidneyAnalyze::AnalyzeImage_OnZoomClarifyButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        // Zoom & Clarify shows a popup with a zoomed image containing "arcadia" text.
        // Show a popup with an embedded image and yes/no options.
        mAnalyzePopup->ResetToDefaults();
        mAnalyzePopup->SetWindowPosition(Vector2(-106.0f, 0.0f));

        mAnalyzePopup->SetTextAlignment(HorizontalAlignment::Center);
        mAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("SaveArcadia"));

        mAnalyzePopup->SetImage(gAssetManager.LoadTexture("POUSSIN_ZOOM.BMP"));

        // This popup has yes/no options.
        // If yes is pressed, we save the "arcadia" text as a file.
        mAnalyzePopup->ShowTwoButton([this](){
            mSidneyFiles->AddFile(SidneyFileIds::kArcadiaText);
            ShowAnalyzeMessage("SavingArcadia", Vector2(), HorizontalAlignment::Center);
        });
    }
}
