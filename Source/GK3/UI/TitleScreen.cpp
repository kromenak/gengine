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

    // The y-position of the buttons on this screen varies based on the screen resolution.
    // While hard to 100% verify, this seems to give close to the correct result.
    float y = 27.0f + (Window::GetSize().y - 480.0f) * 0.0917f;

    // Anchor to bottom-right and position based off that.
    button->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
    button->GetRectTransform()->SetAnchoredPosition(xPos, y);
    return button;
}

TitleScreen::TitleScreen() : Actor(TransformType::RectTransform)
{
    UIUtil::AddColorCanvas(this, 0, Color32::Black);

    // Add title screen image.
    UIImage* titleImage = UIUtil::NewUIActorWithWidget<UIImage>(this);
    titleImage->SetTexture(gAssetManager.LoadTexture("TITLE.BMP"), true);
    titleImage->ResizeToFitPreserveAspect(Window::GetSize());

    // The distance between buttons varies based on screen resolution.
    // Not sure how true this is to the original game, but it gives pretty good-looking results.
    float buttonX = -33.0f;
    float distBetweenButtons = Math::RoundToInt(Math::Min(18.0f + (Window::GetSize().x - 640.0f) * 0.15f, 100.0f));

    // Add "quit" button.
    UIButton* quitButton = CreateButton(titleImage->GetOwner(), "TITLE_QUIT", buttonX);
    quitButton->SetTooltipText("titlequit");
    quitButton->SetPressCallback([](UIButton* button){
        GEngine::Instance()->Quit();
    });
    buttonX -= quitButton->GetRectTransform()->GetSize().x + distBetweenButtons;
    mQuitButton = quitButton;

    // Add "restore" button.
    UIButton* restoreButton = CreateButton(titleImage->GetOwner(), "TITLE_RESTORE", buttonX);
    restoreButton->SetTooltipText("titlerestore");
    restoreButton->SetPressCallback([](UIButton* button){
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        gGK3UI.ShowLoadScreen();
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        restoreButton->SetCanInteract(false);
    }
    buttonX -= restoreButton->GetRectTransform()->GetSize().x + distBetweenButtons;
    mRestoreButton = restoreButton;

    // Add "play" button.
    UIButton* playButton = CreateButton(titleImage->GetOwner(), "TITLE_PLAY", buttonX);
    playButton->SetTooltipText("titleplay");
    playButton->SetPressCallback([this](UIButton* button){
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        Hide();
        GEngine::Instance()->StartGame();
    });
    buttonX -= playButton->GetRectTransform()->GetSize().x + distBetweenButtons;
    mPlayButton = playButton;

    // Add "intro" button.
    UIButton* introButton = CreateButton(titleImage->GetOwner(), "TITLE_INTRO", buttonX);
    introButton->SetTooltipText("titleintro");
    introButton->SetPressCallback([](UIButton* button) {
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
        gVideoPlayer.Play("intro.bik", true, true, nullptr);
    });    
    if(GEngine::Instance()->IsDemoMode())
    {
        introButton->SetCanInteract(false);
    }
    mIntroButton = introButton;
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

void TitleScreen::OnUpdate(float deltaTime)
{
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_I))
    {
        mIntroButton->AnimatePress();
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_P))
    {
        mPlayButton->AnimatePress();
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_R))
    {
        mRestoreButton->AnimatePress();
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_Q))
    {
        mQuitButton->AnimatePress();
    }
}
