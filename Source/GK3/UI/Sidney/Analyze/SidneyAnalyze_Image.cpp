#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "RectTransform.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "Texture.h"
#include "Timers.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "UIVideoImage.h"
#include "VideoPlayer.h"

void SidneyAnalyze::AnalyzeImage_Init()
{
    // Create a parent that contains all the image analysis stuff.
    mAnalyzeImageWindow = new Actor("AnalyzeImageWindow", TransformType::RectTransform);
    mAnalyzeImageWindow->GetTransform()->SetParent(mRoot->GetTransform());
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchor(AnchorPreset::CenterStretch);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchoredPosition(0.0f, 0.0f);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetSizeDelta(0.0f, 0.0f);

    // Create image that is being analyzed.
    {
        mAnalyzeImage = UI::CreateWidgetActor<UIImage>("AnalyzeImage", mAnalyzeImageWindow);
        mAnalyzeImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
    }

    // Create images that are used to render videos in some cases.
    {
        for(UIVideoImage*& image : mAnalyzeVideoImages)
        {
            image = UI::CreateWidgetActor<UIVideoImage>("VideoImage", mAnalyzeImageWindow);
            image->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            AnalyzeImage_ResetVideoImage(image);
        }
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

    // Reset video images to be the size of the analyze image.
    for(UIVideoImage* image : mAnalyzeVideoImages)
    {
        AnalyzeImage_ResetVideoImage(image);
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
        gGameProgress.SetFlag("AnalyzedBasePainting3");
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

void SidneyAnalyze::AnalyzeImage_OnAnalyzeTextPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kParchment2)
    {
        // The game should be in hands-off mode during this short cutscene of "reaching out to the internet" to get data.
        gActionManager.StartManualAction();

        // Play a modem SFX, so it seems like we're reaching out to the internet...
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDMODEM.WAV", AssetScope::Scene));

        // Show a series of messages that make it seem like we're analyzing the text and downloading stuff from the internet.
        ShowAnalyzeMessage("Text1Parch2", Vector2(), HorizontalAlignment::Center, true);
        Timers::AddTimerSeconds(3.0f, [this](){
            ShowAnalyzeMessage("Text2Parch2", Vector2(), HorizontalAlignment::Center, true);

            Timers::AddTimerSeconds(3.0f, [this](){
                ShowAnalyzeMessage("Text3Parch2", Vector2(), HorizontalAlignment::Center, true);

                Timers::AddTimerSeconds(3.0f, [this](){
                    mAnalyzePopup->Hide();

                    // Hands-off mode is done, the user has control again.
                    gActionManager.FinishManualAction();

                    // The last popup asks us to choose a language for extracting a hidden message from the text.
                    // As with the other parchment, the hidden message is in French.
                    mSecondaryAnalyzePopup->ResetToDefaults();
                    mSecondaryAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Text4Parch2"));
                    mSecondaryAnalyzePopup->ShowThreeButton([this](int buttonIndex){
                        if(buttonIndex == 0)
                        {
                            mSecondaryAnalyzePopup->Hide();
                            ShowAnalyzeMessage("Parch2French");
                            gGameProgress.ChangeScore("e_sidney_analysis_anomalies_parch2");

                            if(!gGameProgress.GetFlag("GotThemApples"))
                            {
                                gActionManager.ExecuteDialogueAction("02OCR2Z4L3", 2);
                                gInventoryManager.AddInventoryItem("BLUE_APPLES_RIDDLE");
                                gGameProgress.SetFlag("GotThemApples");
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
                });
            });
        });
    }
}

void SidneyAnalyze::AnalyzeImage_OnViewGeometryButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kParchment1)
    {
        // Play analyze video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("Parch1Geo.avi", mAnalyzeVideoImages[0], "GEOMPARCH1FINAL.BMP", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Show a popup explaining the result.
            ShowAnalyzeMessage("GeometryParch1", Vector2(180.0f, -4.0f), HorizontalAlignment::Center);

            // You have obtained a triangle!
            mSidneyFiles->AddFile(SidneyFileIds::kTriangleShape);

            // Add to score.
            gGameProgress.ChangeScore("e_sidney_analysis_gemoetry_parch1");

            // Add flag.
            gGameProgress.SetFlag("AnalyzedGeomParchment1");
        });
    }
    else if(mAnalyzeFileId == SidneyFileIds::kParchment2)
    {
        // Play analyze video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("Parch2Geo.avi", mAnalyzeVideoImages[0], "GEOMPARCH2FINAL.BMP", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Show a popup explaining the result.
            ShowAnalyzeMessage("GeometryParch2", Vector2(180.0f, -4.0f), HorizontalAlignment::Center);

            // You have obtained a circle and a square!
            mSidneyFiles->AddFile(SidneyFileIds::kCircleShape);
            mSidneyFiles->AddFile(SidneyFileIds::kSquareShape);

            // Add to score.
            gGameProgress.ChangeScore("e_sidney_analysis_gemoetry_parch2");

            // Add flag.
            gGameProgress.SetFlag("AnalyzedGeomParchment2");
        });
    }
    else if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        // For this video, the size and position of the video is unfortunately fairly arbitrary.
        mAnalyzeVideoImages[0]->GetRectTransform()->SetSizeDelta(431.0f, 350.0f);
        Vector2 videoImagePos = mAnalyzeImage->GetRectTransform()->GetAnchoredPosition();
        videoImagePos.x -= 8.0f;
        videoImagePos.y += 68.0f;
        mAnalyzeVideoImages[0]->GetRectTransform()->SetAnchoredPosition(videoImagePos);

        // Play analyze video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("Poussingeo.avi", mAnalyzeVideoImages[0], "GEOMPOUSSINFINAL.BMP", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Show a popup explaining the result.
            ShowAnalyzeMessage("GeometryPous", Vector2(180.0f, 0.0f), HorizontalAlignment::Center);

            // You get a hexagram!
            mSidneyFiles->AddFile(SidneyFileIds::kHexagramShape);

            // Add to score.
            gGameProgress.ChangeScore("e_sidney_analysis_gemoetry_poussin");

            // Add flags.
            gGameProgress.SetFlag("AnalyzedGeomPainting1");
            gGameProgress.SetFlag("SavedHexagram");
        });
    }
    else if(mAnalyzeFileId == SidneyFileIds::kTeniersPostcard2)
    {
        for(int i = 0; i < 2; ++i)
        {
            mAnalyzeVideoImages[i]->GetRectTransform()->SetSizeDelta(464.0f, 350.0f);
            Vector2 videoImagePos = mAnalyzeImage->GetRectTransform()->GetAnchoredPosition();
            videoImagePos.y += 20.0f;
            mAnalyzeVideoImages[i]->GetRectTransform()->SetAnchoredPosition(videoImagePos);
        }

        // Says "analyzing..."
        ShowAnalyzeMessage("GeometryTenier1", Vector2(190.0f, -160.0f), HorizontalAlignment::Center, true);

        // Play initial video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("TenierGeoA.avi", mAnalyzeVideoImages[0], "TENIERGEOA.BMP", Color32(0, 255, 0), [this](){

            // Says something about the result of the first video.
            ShowAnalyzeMessage("GeometryTenier2", Vector2(190.0f, -160.0f), HorizontalAlignment::Center, true);
            Timers::AddTimerSeconds(4.0f, [this](){

                // Play another video.
                AnalyzeImage_PlayVideo("TenierGeob.avi", mAnalyzeVideoImages[0], "TENIERGEOB.BMP", Color32(0, 255, 0), [this](){

                    // Says something about the result of the second video.
                    ShowAnalyzeMessage("GeometryTenier3", Vector2(190.0f, -160.0f), HorizontalAlignment::Center, true);
                    Timers::AddTimerSeconds(4.0f, [this](){

                        // Show tilted square.
                        mAnalyzeVideoImages[0]->SetTexture(gAssetManager.LoadTexture("TENIERGEOC.BMP", AssetScope::Scene));
                        mAnalyzeVideoImages[0]->GetTexture()->SetTransparentColor(Color32(0, 255, 0));
                        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTTON4.WAV"));
                        ShowAnalyzeMessage("GeometryTenier4", Vector2(190.0f, -160.0f), HorizontalAlignment::Center, true);

                        // Wait a bit more.
                        Timers::AddTimerSeconds(4.0f, [this](){

                            // Final video.
                            AnalyzeImage_PlayVideo("TenierGeoD.avi", mAnalyzeVideoImages[1], "GEOMTENNIERSFINAL.BMP", Color32::Magenta, [this](){
                                gActionManager.FinishManualAction();
                                ShowAnalyzeMessage("GeometryTenier5", Vector2(190.0f, -164.0f), HorizontalAlignment::Center);

                                // Grace says "I already know about the tilted square!"
                                // You don't even get any points for all this work!
                                if(!gGameProgress.GetFlag("AnalyzedGeomPainting3"))
                                {
                                    gActionManager.ExecuteDialogueAction("02O2F2ZQ37", 2);
                                    gGameProgress.SetFlag("AnalyzedGeomPainting3");
                                }
                            });
                        });
                    });
                });
            });
        });
    }

    // The "view geometry" button becomes disabled if the viewed geometry is already visible on-screen.
    // We always use video image 0 for this, so we can disable the choice if the video image is enabled.
    // This gets reset if you open a different file.
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, !mAnalyzeVideoImages[0]->IsEnabled());
}

void SidneyAnalyze::AnalyzeImage_OnRotateShapeButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kParchment2)
    {
        // Play zoom in video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("parch2zoom.avi", mAnalyzeVideoImages[2], "", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Turn off video image once this video finishes.
            mAnalyzeVideoImages[2]->SetEnabled(false);

            // Show an image popup with the rotated image.
            mAnalyzePopup->ResetToDefaults();
            mAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("RotateParch2"));
            mAnalyzePopup->SetImage(gAssetManager.LoadTexture("ZION_ROT.BMP", AssetScope::Scene));
            mAnalyzePopup->SetTextAlignment(HorizontalAlignment::Center);
            mAnalyzePopup->SetWindowPosition(Vector2(-112.0f, 0.0f));
            mAnalyzePopup->SetWindowSize(Vector2(135.0f, 243.0f));
            mAnalyzePopup->ShowOneButton();

            // Add to score.
            gGameProgress.ChangeScore("e_sidney_analysis_view_rotation_parch2");

            // Play some dialogue.
            gActionManager.ExecuteDialogueAction("02OEV2Z4L1");
        });
    }
}

void SidneyAnalyze::AnalyzeImage_OnZoomClarifyButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kPoussinPostcard)
    {
        // Play zoom in video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("poussinzoom.avi", mAnalyzeVideoImages[2], "", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Turn off video image once this video finishes.
            mAnalyzeVideoImages[2]->SetEnabled(false);

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
                gGameProgress.SetFlag("SavedArcadiaText");

                // Show popup confirming saved text.
                // This one needs some custom code to deal with callback on OK press.
                mAnalyzePopup->ResetToDefaults();
                mAnalyzePopup->SetTextAlignment(HorizontalAlignment::Center);
                mAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("SavingArcadia"));
                mAnalyzePopup->ShowOneButton([](){
                    // This might end the timeblock.
                    SidneyUtil::CheckForceExitSidney307A();
                });
            });

            // Add to score.
            gGameProgress.ChangeScore("e_sidney_analysis_view_words_poussin");
        });
    }
    else if(mAnalyzeFileId == SidneyFileIds::kTeniersPostcard2)
    {
        // Play zoom in video.
        gActionManager.StartManualAction();
        AnalyzeImage_PlayVideo("Tenierzoom.avi", mAnalyzeVideoImages[2], "", Color32::Magenta, [this](){
            gActionManager.FinishManualAction();

            // Turn off video image once this video finishes.
            mAnalyzeVideoImages[2]->SetEnabled(false);

            // Zoom & Clarify shows a popup with a zoomed image containing a bible page.
            // Show a popup with an embedded image and yes/no options.
            mAnalyzePopup->ResetToDefaults();
            mAnalyzePopup->SetWindowPosition(Vector2(-78.0f, 37.0f));

            mAnalyzePopup->SetTextAlignment(HorizontalAlignment::Center);
            mAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("GetVerse"));

            mAnalyzePopup->SetImage(gAssetManager.LoadTexture("TENIERS_ZOOM.BMP"));

            // This popup has yes/no options.
            // If yes is pressed, we "reach out to the internet" to get the text.
            mAnalyzePopup->ShowTwoButton([this](){

                // Play a modem SFX, so it seems like we're reaching out to the internet...
                gActionManager.StartManualAction();
                gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDMODEM.WAV", AssetScope::Scene));
                ShowAnalyzeMessage("RetrieveVerse", Vector2(), HorizontalAlignment::Center, true);

                // The audio file is about six seconds long.
                Timers::AddTimerSeconds(6.0f, [this](){

                    // Show the verse and play some dialogue.
                    // There are no points or flags associated with this action though.
                    gActionManager.FinishManualAction();
                    ShowAnalyzeMessage("Verse", Vector2(), HorizontalAlignment::Left);
                    gActionManager.ExecuteDialogueAction("02OCB2ZQ35", 1);
                });
            });

            // Add to score just for zooming in.
            gGameProgress.ChangeScore("e_sidney_analysis_view_words_tenier");
        });
    }
}

void SidneyAnalyze::AnalyzeImage_ResetVideoImage(UIVideoImage* image)
{
    image->GetRectTransform()->SetAnchoredPosition(mAnalyzeImage->GetRectTransform()->GetAnchoredPosition());
    image->GetRectTransform()->SetSizeDelta(mAnalyzeImage->GetRectTransform()->GetSizeDelta());
    image->SetEnabled(false);
}

void SidneyAnalyze::AnalyzeImage_PlayVideo(const std::string& videoName, UIVideoImage* videoImage, const std::string& finalTextureName, Color32 finalTextureTransparentColor, const std::function<void()>& finishCallback)
{
    // Make sure image is enabled.
    videoImage->SetEnabled(true);

    // Play a manual action during the movie.
    gActionManager.StartManualAction();

    // Play video on video image. The video uses a green chromakey background.
    Color32 transparentColor(3, 251, 3);
    videoImage->Play(videoName, transparentColor, [videoImage, finalTextureName, finalTextureTransparentColor, finishCallback](){

        // Manual action is done.
        gActionManager.FinishManualAction();

        // Set a final texture if specified.
        Texture* finalTexture = gAssetManager.LoadTexture(finalTextureName, AssetScope::Scene);
        if(finalTexture != nullptr)
        {
            videoImage->SetTransparentColor(finalTextureTransparentColor);
            videoImage->SetTexture(finalTexture);
        }

        // Execute callback.
        finishCallback();
    });
}