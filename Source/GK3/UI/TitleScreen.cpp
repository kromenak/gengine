#include "TitleScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "GK3UI.h"
#include "GKPrefs.h"
#include "InputManager.h"
#include "SoundtrackPlayer.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

namespace
{
    UIButton* CreateButton(Component* parent, const std::string& buttonId)
    {
        UIButton* button = UI::CreateWidgetActor<UIButton>(buttonId, parent);

        // Set textures.
        button->SetUpTexture(gAssetManager.LoadTexture(buttonId + "_U.BMP"));
        button->SetDownTexture(gAssetManager.LoadTexture(buttonId + "_D.BMP"));
        button->SetHoverTexture(gAssetManager.LoadTexture(buttonId + "_H.BMP"));
        button->SetDisabledTexture(gAssetManager.LoadTexture(buttonId + "_X.BMP"));

        // Anchor to bottom-right and position based off that.
        button->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        return button;
    }
}

TitleScreen::TitleScreen() : Actor("TitleScreen", TransformType::RectTransform)
{
    mCanvas = UI::AddCanvas(this, 0, Color32::Black);

    // Add title screen image.
    mBackgroundImage = UI::CreateWidgetActor<UIImage>("Background", this);
    mBackgroundImage->SetTexture(gAssetManager.LoadTexture("TITLE.BMP"), true);

    // Add "quit" button.
    UIButton* quitButton = CreateButton(mBackgroundImage, "TITLE_QUIT");
    quitButton->SetTooltipText("titlequit");
    quitButton->SetPressCallback([](UIButton* button){
        // Unlike most other quit buttons in the game, this one quits directly - no quit popup confirmation.
        GEngine::Instance()->Quit();
    });
    mQuitButton = quitButton;

    // Add "restore" button.
    UIButton* restoreButton = CreateButton(mBackgroundImage, "TITLE_RESTORE");
    restoreButton->SetTooltipText("titlerestore");
    restoreButton->SetPressCallback([](UIButton* button){
        gGK3UI.ShowLoadScreen();
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        restoreButton->SetCanInteract(false);
    }
    mRestoreButton = restoreButton;

    // Add "play" button.
    UIButton* playButton = CreateButton(mBackgroundImage, "TITLE_PLAY");
    playButton->SetTooltipText("titleplay");
    playButton->SetPressCallback([this](UIButton* button){
        Hide();
        GEngine::Instance()->StartGame();
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    });
    mPlayButton = playButton;

    // Add "intro" button.
    UIButton* introButton = CreateButton(mBackgroundImage, "TITLE_INTRO");
    introButton->SetTooltipText("titleintro");
    introButton->SetPressCallback([](UIButton* button) {
        gGK3UI.GetVideoPlayer()->Play("intro.bik", true, true, nullptr);
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    });
    if(GEngine::Instance()->IsDemoMode())
    {
        introButton->SetCanInteract(false);
    }
    mIntroButton = introButton;

    // Make sure UI scale and button positions are correct for current resolution.
    RefreshUIScaling();
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
    if(!IsActive()) { return; }
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
    // Each button on the title screen has a keyboard shortcut.
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

    // Keep UI scale correct if resolution changes.
    RefreshUIScaling();
}

void TitleScreen::RefreshUIScaling()
{
    // The authored size of the background image is 640x480.
    Vector2 bgImageSize(640.0f, 480.0f);

    // If we want to use the original game's scaling logic, we can do so.
    // But this logic breaks down after ~1080p (in fact, it was only ever defined up to 1024x768).
    // After that, we should fall back on the auto scaling logic.
    if(Prefs::UseOriginalUIScalingLogic() && Window::GetHeight() < Prefs::GetMinimumScaleUIHeight())
    {
        // Turn off canvas autoscaling. This sets canvas scale to 1, and width/height equal to window width/height.
        mCanvas->SetAutoScale(false);

        // Resize background image to fit within window size, preserving aspect ratio.
        mBackgroundImage->ResizeToFitPreserveAspect(Window::GetSize());

        // The background image size is now whatever was calculated.
        bgImageSize = mBackgroundImage->GetRectTransform()->GetSizeDelta();
    }
    else // not using original game's logic.
    {
        // In this case, just use 640x480 and have it auto-scale when the resolution gets too big.
        mCanvas->SetAutoScale(true);
        mBackgroundImage->ResizeToTexture();
    }

    // The logic here positions the buttons correctly for 640x480, 800x600, and 1024x768 resolutions.
    // For larger resolutions (up to 1080p), it extrapolates and the results are pretty good.
    // Beyond that, we revert to 640x480 (with autoscaling) anyways, so the result is still good.

    // The y-position of the buttons on this screen varies based on the background image height.
    // While hard to 100% verify, this seems to give close to the correct result.
    float buttonY = 27.0f + (bgImageSize.y - 480.0f) * 0.0917f;

    // Math for button x-pos and distances between buttons.
    // Again, hard to say if this is anywhere near how the original game actually did it, but it gives very similar results.
    float buttonX = -33.0f - (bgImageSize.y - 480.0f) * 0.16f;
    float distBetweenButtons = Math::RoundToInt(Math::Min(18.0f + (bgImageSize.x - 640.0f) * 0.15f, 100.0f));

    mQuitButton->GetRectTransform()->SetAnchoredPosition(buttonX, buttonY);
    buttonX -= mQuitButton->GetRectTransform()->GetSize().x + distBetweenButtons;

    mRestoreButton->GetRectTransform()->SetAnchoredPosition(buttonX, buttonY);
    buttonX -= mRestoreButton->GetRectTransform()->GetSize().x + distBetweenButtons;

    mPlayButton->GetRectTransform()->SetAnchoredPosition(buttonX, buttonY);
    buttonX -= mPlayButton->GetRectTransform()->GetSize().x + distBetweenButtons;

    mIntroButton->GetRectTransform()->SetAnchoredPosition(buttonX, buttonY);
    buttonX -= mPlayButton->GetRectTransform()->GetSize().x + distBetweenButtons;
}
