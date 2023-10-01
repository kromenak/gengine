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
    AddComponent<UICanvas>(-1);

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
                mAnalyze.Show();
            }
        });

        buttonPos += kButtonSpacing;
        UIButton* translateButton = CreateMainButton(desktopBackground, "TRANSL", buttonPos);
        translateButton->SetPressCallback([](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            printf("Translate\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* dataButton = CreateMainButton(desktopBackground, "ADDATA", buttonPos);
        dataButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mAddData.Start();
        });

        buttonPos += kButtonSpacing;
        UIButton* idButton = CreateMainButton(desktopBackground, "MAKEID", buttonPos);
        idButton->SetPressCallback([this](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            mMakeId.Show();
        });

        buttonPos += kButtonSpacing;
        UIButton* suspectsButton = CreateMainButton(desktopBackground, "SUSPT", buttonPos);
        suspectsButton->SetPressCallback([](UIButton* button){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDENTER.WAV"));
            printf("Suspects\n");
        });
    }

    // Add "New Email" label.
    {
        Actor* newEmailActor = new Actor(TransformType::RectTransform);
        newEmailActor->GetTransform()->SetParent(desktopBackground->GetTransform());
        mNewEmailLabel = newEmailActor->AddComponent<UILabel>();

        mNewEmailLabel->SetFont(gAssetManager.LoadFont("SID_PDN_10_GRN.FON"));
        mNewEmailLabel->SetText("NEW E-MAIL");
        mNewEmailLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mNewEmailLabel->SetVerticalAlignment(VerticalAlignment::Top);

        mNewEmailLabel->GetRectTransform()->SetPivot(1.0f, 1.0f);
        mNewEmailLabel->GetRectTransform()->SetAnchor(1.0f, 1.0f);
        mNewEmailLabel->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        mNewEmailLabel->GetRectTransform()->SetSizeDelta(100.0f, 20.0f);
    }

    // Create subscreens.
    mSearch.Init(this);
    mEmail.Init(this);
    mAnalyze.Init(this, &mFiles);
    mAddData.Init(this, &mFiles);
    mMakeId.Init(this);

    mFiles.Init(this);
    // Not active by default.
    SetActive(false);
}

void Sidney::Show()
{
    // Show Sidney UI.
    SetActive(true);

    //TODO: We'll assume there is ALWAYS new email for the moment...
    bool newEmail = true;

    // Hide the "new email" label by default, regardless of new email state.
    // If we DO have new email, this will blink on in a moment.
    mNewEmailLabel->SetEnabled(false);

    // If no new email, set this timer to -1. The label will never appear.
    // Otherwise, set to blink interval to have it blink on and off.
    mNewEmailBlinkTimer = newEmail ? kNewEmailBlinkInterval : -1;

    // If there is new email, play the "new email" audio cue.
    mPlayNewEmailSfx = newEmail;
}

void Sidney::Hide()
{
    // Hide Sidney UI.
    SetActive(false);

    // Whenever you exit Sidney, no matter where you are in the game, you warp to R25.
    // This makes sense under the assumption that you only access Sidney in R25 anyway!
    gLocationManager.ChangeLocation("R25", [](){

        // This special function warps Ego to the "sitting at desk" position and plays the stand up animation.
        gActionManager.ExecuteSheepAction("R25_ALL", "ExitSidney$");
    });
}

bool Sidney::HasFile(const std::string& fileName)
{
    return mFiles.HasFile(fileName);
}

void Sidney::OnUpdate(float deltaTime)
{
    // When adding data, Sidney should not update.
    // We mainly wait for the inventory layer to be closed.
    mAddData.OnUpdate(deltaTime);
    if(mAddData.AddingData())
    {
        return;
    }

    // Play "New Email" SFX the first chance we get.
    // If we do this during an action skip, the action skip logic will stomp this audio. So, wait until no skip is happening.
    if(mPlayNewEmailSfx && !gActionManager.IsSkippingCurrentAction())
    {
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("NEWEMAIL.WAV"));
        mPlayNewEmailSfx = false;
    }

    // Track timer countdown for new email to blink in the corner.
    if(mNewEmailBlinkTimer > 0.0f)
    {
        mNewEmailBlinkTimer -= deltaTime;
        if(mNewEmailBlinkTimer <= 0.0f)
        {
            mNewEmailLabel->SetEnabled(!mNewEmailLabel->IsEnabled());
            mNewEmailBlinkTimer = kNewEmailBlinkInterval;
        }
    }

    // Update each screen in turn.
    // Each screen will early out if not active.
    mSearch.OnUpdate(deltaTime);
    mAnalyze.OnUpdate(deltaTime);
    mMakeId.OnUpdate(deltaTime);
}