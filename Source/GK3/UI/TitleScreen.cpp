#include "TitleScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "SoundtrackPlayer.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "VideoPlayer.h"

static UIButton* CreateButton(Actor* parent, const std::string& buttonId, float xPos)
{
    Actor* buttonActor = new Actor(TransformType::RectTransform);
    buttonActor->GetTransform()->SetParent(parent->GetTransform());
    UIButton* button = buttonActor->AddComponent<UIButton>();

    // Set textures.
    button->SetUpTexture(gAssetManager.LoadTexture(buttonId + "_U.BMP"));
    button->SetDownTexture(gAssetManager.LoadTexture(buttonId + "_D.BMP"));
    button->SetHoverTexture(gAssetManager.LoadTexture(buttonId + "_H.BMP"));
    button->SetDisabledTexture(gAssetManager.LoadTexture(buttonId + "_X.BMP"));

    // Anchor to bottom-right and position based off that.
    button->GetRectTransform()->SetAnchor(1.0f, 0.0f);
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchoredPosition(xPos, 67.0f);
    return button;
}

TitleScreen::TitleScreen() : Actor(TransformType::RectTransform)
{
    AddComponent<UICanvas>(0);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add title screen background image.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(gAssetManager.LoadTexture("TITLE.BMP"));

    // Add "intro" button.
    UIButton* introButton = CreateButton(this, "TITLE_INTRO", -505.0f);
    introButton->SetPressCallback([](UIButton* button) {
        gVideoPlayer.Play("intro.bik", true, true, nullptr);
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        introButton->SetCanInteract(false);
    }

    // Add "play" button.
    UIButton* playButton = CreateButton(this, "TITLE_PLAY", -381.0f);
    playButton->SetPressCallback([this](UIButton* button) {
        Hide();
        GEngine::Instance()->StartGame();
    });

    // Add "restore" button.
    UIButton* restoreButton = CreateButton(this, "TITLE_RESTORE", -257.0f);
    restoreButton->SetPressCallback([](UIButton* button) {
        std::cout << "Restore!" << std::endl;
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        restoreButton->SetCanInteract(false);
    }

    // Add "quit" button.
    UIButton* quitButton = CreateButton(this, "TITLE_QUIT", -135.0f);
    quitButton->SetPressCallback([](UIButton* button) {
        GEngine::Instance()->Quit();
    });
}

void TitleScreen::Show()
{
    SetActive(true);
    
    // Play theme music via soundtrack system.
    SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
    if(soundtrackPlayer == nullptr)
    {
        soundtrackPlayer = AddComponent<SoundtrackPlayer>();
    }
    soundtrackPlayer->Play(gAssetManager.LoadSoundtrack("TITLETHEME.STK"));
}

void TitleScreen::Hide()
{
    SetActive(false);

    // Fade out theme music.
    SoundtrackPlayer* soundtrackPlayer = GetComponent<SoundtrackPlayer>();
    if(soundtrackPlayer != nullptr)
    {
        soundtrackPlayer->StopAll();
    }
}