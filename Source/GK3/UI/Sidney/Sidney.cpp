#include "Sidney.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Scene.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

namespace
{
    UIButton* CreateMainButton(Actor* parent, const std::string& buttonId, float xPos)
    {
        Actor* actor = new Actor(TransformType::RectTransform);
        actor->GetTransform()->SetParent(parent->GetTransform());
        UIButton* button = actor->AddComponent<UIButton>();

        button->GetRectTransform()->SetPivot(0.0f, 1.0f);
        button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
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
    AddComponent<UICanvas>(-2);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add black background that eats input.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(&Texture::Black);
    background->SetReceivesInput(true);

    // Add desktop background image.
    Actor* desktopBackground = new Actor(TransformType::RectTransform);
    desktopBackground->GetTransform()->SetParent(GetTransform());
    UIImage* desktopBackgroundImage = desktopBackground->AddComponent<UIImage>();
    desktopBackgroundImage->SetTexture(gAssetManager.LoadTexture("S_MAIN_SCN.BMP"), true);

    // Add exit button as child of desktop background.
    {
        SidneyButton* button = new SidneyButton(desktopBackground);
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
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mFiles.Show();
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
    SetActive(false);

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