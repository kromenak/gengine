#include "Sidney.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Scene.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIUtil.h"
#include "Window.h"

namespace
{
    UIButton* CreateMainButton(Actor* parent, const std::string& buttonId, float xPos)
    {
        UIButton* button = UI::CreateWidgetActor<UIButton>(buttonId, parent);
        button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        button->GetRectTransform()->SetAnchoredPosition(xPos, -24.0f);

        button->SetUpTexture(gAssetManager.LoadTexture("B_" + buttonId + "_U.BMP"));
        button->SetHoverTexture(gAssetManager.LoadTexture("B_" + buttonId + "_H.BMP"));
        button->SetDownTexture(gAssetManager.LoadTexture("B_" + buttonId + "_D.BMP"));
        button->SetDisabledTexture(gAssetManager.LoadTexture("B_" + buttonId + "_X.BMP"));
        return button;
    }
}

Sidney::Sidney() : Actor("Sidney", TransformType::RectTransform)
{
    // Sidney will be layered near the bottom.
    // A lot of stuff needs to appear above it (inventory, status overlay, etc).
    UI::AddCanvas(this, 0, Color32::Black);

    // Add desktop background image.
    UIImage* desktopBackgroundImage = UI::CreateWidgetActor<UIImage>("Desktop", this);
    desktopBackgroundImage->SetTexture(gAssetManager.LoadTexture("S_MAIN_SCN.BMP"), true);
    Actor* desktopBackground = desktopBackgroundImage->GetOwner();

    // Add exit button as child of desktop background.
    {
        SidneyButton* button = new SidneyButton("ExitButton", desktopBackground);
        button->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"));
        button->SetText(SidneyUtil::GetMainScreenLocalizer().GetText("MenuItem9"));
        button->SetWidth(80.0f);

        button->SetPressAudio(gAssetManager.LoadAudio("SIDBUTTON3.WAV"));
        button->SetPressCallback([this](){
            Hide();
        });

        button->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        button->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        button->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
    }

    // Add button bar for subscreens.
    {
        const float kButtonStart = 5.0f;
        const float kButtonSpacing = 79.0f;

        float buttonPos = kButtonStart;
        UIButton* searchButton = CreateMainButton(desktopBackground, "SEARCH", buttonPos);
        searchButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));

            // Gabe refuses to use the search system.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O6I2ZQR1\", 1)");
            }
            else
            {
                mFiles.HideAllFileWindows();
                mSearch.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* emailButton = CreateMainButton(desktopBackground, "EMAIL", buttonPos);
        emailButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));

            // Gabe also doesn't want to use email.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O1E2ZQR1\", 1)");
            }
            else
            {
                mFiles.HideAllFileWindows();
                mEmail.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* filesButton = CreateMainButton(desktopBackground, "FILES", buttonPos);
        filesButton->SetPressCallback([this](UIButton* button){

            // Show file selector, along with button SFX.
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.Show([this](SidneyFile* selectedFile){

                // When a file is clicked, try to direct to the most relevant area of Sidney, with that file opened.
                // Images => go to analyze with that file opened.
                // Text/Audio => go to translate with that file opened.
                // Fingerprints/Plates => go to suspects with that file opened.
                switch(selectedFile->type)
                {
                    case SidneyFileType::Image:
                        mAnalyze.Show(selectedFile->id);
                        break;

                    case SidneyFileType::Text:
                    case SidneyFileType::Audio:
                        mTranslate.Show(selectedFile->id);
                        break;

                    case SidneyFileType::License:
                    case SidneyFileType::Fingerprint:
                        mSuspects.Show();
                        mSuspects.OpenFile(selectedFile->id);
                        break;

                    case SidneyFileType::Shape:
                    default:
                        // Do nothing for anything else.
                        break;
                }

                // Plays another button SFX upon selecting a file.
                gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            });
        });

        buttonPos += kButtonSpacing;
        UIButton* analyzeButton = CreateMainButton(desktopBackground, "ANALYZE", buttonPos);
        analyzeButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));

            // Gabe also doesn't want to analyze stuff.
            if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
            {
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O7A2ZQR1\", 1)");
            }
            else
            {
                mFiles.HideAllFileWindows();
                mAnalyze.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* translateButton = CreateMainButton(desktopBackground, "TRANSL", buttonPos);
        translateButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.HideAllFileWindows();
            mTranslate.Show();
        });

        buttonPos += kButtonSpacing;
        UIButton* dataButton = CreateMainButton(desktopBackground, "ADDATA", buttonPos);
        dataButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.HideAllFileWindows();
            mAddData.Start();
        });

        buttonPos += kButtonSpacing;
        UIButton* idButton = CreateMainButton(desktopBackground, "MAKEID", buttonPos);
        idButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.HideAllFileWindows();
            mMakeId.Show();
        });

        buttonPos += kButtonSpacing;
        UIButton* suspectsButton = CreateMainButton(desktopBackground, "SUSPT", buttonPos);
        suspectsButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.HideAllFileWindows();
            mSuspects.Show();
        });
    }

    // Create subscreens.
    mSearch.Init(this);
    mEmail.Init(this, desktopBackground);
    mAnalyze.Init(this, &mFiles, &mTranslate);
    mTranslate.Init(this, &mFiles);
    mAddData.Init(this, &mFiles);
    mMakeId.Init(this);
    mSuspects.Init(this, &mFiles);

    mFiles.Init(this);

    // Add room background images around the desktop.
    {
        UIImage* topImage = UI::CreateWidgetActor<UIImage>("BGTop", desktopBackgroundImage);
        topImage->GetRectTransform()->SetAnchor(AnchorPreset::Top);
        topImage->GetRectTransform()->SetPivot(0.5f, 0.0f);
        topImage->SetTexture(gAssetManager.LoadTexture("S_SID_BKGD1024_TOP_A.BMP"), true);

        UIImage* bottomImage = UI::CreateWidgetActor<UIImage>("BGBottom", desktopBackgroundImage);
        bottomImage->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        bottomImage->GetRectTransform()->SetPivot(0.5f, 1.0f);
        bottomImage->SetTexture(gAssetManager.LoadTexture("S_SID_BKGD1024_BOTTOM_A.BMP"), true);

        UIImage* leftImage = UI::CreateWidgetActor<UIImage>("BGLeft", desktopBackgroundImage);
        leftImage->GetRectTransform()->SetAnchor(AnchorPreset::Left);
        leftImage->GetRectTransform()->SetPivot(1.0f, 0.5f);
        leftImage->SetTexture(gAssetManager.LoadTexture("S_SID_BKGD1024_LEFT_A.BMP"), true);

        UIImage* rightImage = UI::CreateWidgetActor<UIImage>("BGRight", desktopBackgroundImage);
        rightImage->GetRectTransform()->SetAnchor(AnchorPreset::Right);
        rightImage->GetRectTransform()->SetPivot(0.0f, 0.5f);
        rightImage->SetTexture(gAssetManager.LoadTexture("S_SID_BKGD1024_RIGHT_A.BMP"), true);

        mLamaImage = UI::CreateWidgetActor<UIImage>("BGOverlay", desktopBackgroundImage);
        mLamaImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mLamaImage->SetTexture(gAssetManager.LoadTexture("S_SID_BKGD800_LAMA_A.BMP"), true);
    }

    // Not active by default.
    SetActive(false);
}

void Sidney::Show()
{
    // Show Sidney UI.
    SetActive(true);

    // Check for whether we should play the "New Email" SFX.
    mEmail.CheckNewEmail();
}

void Sidney::Hide()
{
    // Hide Sidney UI.
    if(!IsActive()) { return; }
    SetActive(false);

    // It's possible to exit Sidney with the file window still up. If so, close it so it's not open when Sidney is next opened.
    mFiles.HideAllFileWindows();

    // Make sure all subscreens are hidden.
    mSearch.Hide();
    mEmail.Hide();
    mAnalyze.Hide();
    mTranslate.Hide();
    mMakeId.Hide();
    mSuspects.Hide();

    // Whenever you exit Sidney, no matter where you are in the game, you warp to R25.
    // This makes sense under the assumption that you only access Sidney in R25 anyway!
    gLocationManager.ChangeLocation("R25", [](){

        // This special function warps Ego to the "sitting at desk" position and plays the stand up animation.
        // It's also used to execute certain logic where exiting Sidney ends a timeblock or plays some other animation dependent on game state.
        gActionManager.ExecuteSheepAction("R25_ALL", "ExitSidney$");
    });
}

bool Sidney::HasFile(const std::string& fileName)
{
    return mFiles.HasFile(fileName);
}

void Sidney::OnPersist(PersistState& ps)
{
    mFiles.OnPersist(ps);
    mAnalyze.OnPersist(ps);

    // Some save data wasn't included in the original save format.
    if(ps.GetFormatVersionNumber() >= 2)
    {
        mSearch.OnPersist(ps);
        mEmail.OnPersist(ps);
        mTranslate.OnPersist(ps);
        mSuspects.OnPersist(ps);
    }
}

void Sidney::OnUpdate(float deltaTime)
{
    if(!IsActive()) { return; }

    // This is a bit wasteful, but update the lama image based on screen resolution.
    // It's rare to change resolution in Sidney, but if you do, we want to make sure this reacts.
    mLamaImage->SetEnabled(Window::GetHeight() > 480);

    // We want to keep the "New Email" label updating at all times.
    mEmail.UpdateNewEmail(deltaTime);

    // When adding data, Sidney should not update.
    // We mainly wait for the inventory layer to be closed.
    mAddData.OnUpdate(deltaTime);
    if(mAddData.AddingData())
    {
        return;
    }

    // Update each screen in turn.
    // Each screen will early out if not active.
    mSearch.OnUpdate(deltaTime);
    mAnalyze.OnUpdate(deltaTime);
    mTranslate.OnUpdate(deltaTime);
    mMakeId.OnUpdate(deltaTime);
    mSuspects.OnUpdate(deltaTime);
}