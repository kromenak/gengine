#include "TitleScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "GK3UI.h"
#include "SoundtrackPlayer.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "VideoPlayer.h"
#include "Window.h"

static UIButton* CreateButton(Actor* parent, const std::string& buttonId, float xPos)
{
    UIButton* button = UIUtil::NewUIActorWithWidget<UIButton>(parent);

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
    UIUtil::AddColorCanvas(this, 0, Color32::Black);

    // Add title screen image.
    UIImage* titleImage = UIUtil::NewUIActorWithWidget<UIImage>(this);
    titleImage->SetTexture(gAssetManager.LoadTexture("TITLE.BMP"), true);
    titleImage->ResizeToFitPreserveAspect(Window::GetSize());
   
    // Add "intro" button.
    UIButton* introButton = CreateButton(titleImage->GetOwner(), "TITLE_INTRO", -505.0f);
    introButton->SetPressCallback([](UIButton* button) {
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        gVideoPlayer.Play("intro.bik", true, true, nullptr);
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        introButton->SetCanInteract(false);
    }

    // Add "play" button.
    UIButton* playButton = CreateButton(titleImage->GetOwner(), "TITLE_PLAY", -381.0f);
    playButton->SetPressCallback([this](UIButton* button) {
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        Hide();
        GEngine::Instance()->StartGame();
    });

    // Add "restore" button.
    UIButton* restoreButton = CreateButton(titleImage->GetOwner(), "TITLE_RESTORE", -257.0f);
    restoreButton->SetPressCallback([](UIButton* button) {
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        gGK3UI.ShowLoadScreen();
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        restoreButton->SetCanInteract(false);
    }

    // Add "quit" button.
    UIButton* quitButton = CreateButton(titleImage->GetOwner(), "TITLE_QUIT", -135.0f);
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