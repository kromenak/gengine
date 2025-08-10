#include "DeathScreen.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "GK3UI.h"
#include "GKPrefs.h"
#include "LocationManager.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

DeathScreen::DeathScreen() : Actor("Death Screen", TransformType::RectTransform),
    mLayer("DeathLayer")
{
    mLayer.OverrideAudioState(true);

    // Canvas takes up the entire screen.
    mCanvas = UI::AddCanvas(this, 20, Color32::Black);

    // Add death screen background image.
    mBackgroundImage = UI::CreateWidgetActor<UIImage>("Background", this);
    mBackgroundImage->SetTexture(gAssetManager.LoadTexture("DEATHSCREEN.BMP"), true);

    // Add buttons for retry, replay, and quit.
    {
        UIButton* retryButton = UI::CreateWidgetActor<UIButton>("RetryButton", mBackgroundImage);
        retryButton->SetUpTexture(gAssetManager.LoadTexture("DS_RTRY_N.BMP"));
        retryButton->SetHoverTexture(gAssetManager.LoadTexture("DS_RTRY_H.BMP"));
        retryButton->SetDownTexture(gAssetManager.LoadTexture("DS_RTRY_D.BMP"));
        retryButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_RTRY_X.BMP"));
        retryButton->SetPressCallback([this](UIButton* button){
            OnRetryButtonPressed();
        });
        retryButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        retryButton->GetRectTransform()->SetAnchoredPosition(-40.0f - 18.0f - 40.0f, 24.0f);
        mRetryButton = retryButton;
    }
    {
        UIButton* restoreButton = UI::CreateWidgetActor<UIButton>("RestoreButton", mBackgroundImage);
        restoreButton->SetUpTexture(gAssetManager.LoadTexture("DS_REST_N.BMP"));
        restoreButton->SetHoverTexture(gAssetManager.LoadTexture("DS_REST_H.BMP"));
        restoreButton->SetDownTexture(gAssetManager.LoadTexture("DS_REST_D.BMP"));
        restoreButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_REST_X.BMP"));
        restoreButton->SetPressCallback([](UIButton* button){
            gGK3UI.ShowLoadScreen();
        });
        restoreButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        restoreButton->GetRectTransform()->SetAnchoredPosition(0.0f, 24.0f);
        mRestoreButton = restoreButton;
    }
    {
        UIButton* quitButton = UI::CreateWidgetActor<UIButton>("QuitButton", mBackgroundImage);
        quitButton->SetUpTexture(gAssetManager.LoadTexture("DS_QUIT_N.BMP"));
        quitButton->SetHoverTexture(gAssetManager.LoadTexture("DS_QUIT_H.BMP"));
        quitButton->SetDownTexture(gAssetManager.LoadTexture("DS_QUIT_D.BMP"));
        quitButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_QUIT_X.BMP"));
        quitButton->SetPressCallback([](UIButton* button){
            gGK3UI.ShowQuitPopup();
        });
        quitButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        quitButton->GetRectTransform()->SetAnchoredPosition(40.0f + 18.0f + 40.0f, 24.0f);
        mQuitButton = quitButton;
    }

    // Not active by default.
    SetActive(false);
}

void DeathScreen::Show()
{
    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);

    // Make sure UI is scaled to match resolution.
    RefreshUIScaling();

    // Play death stinger sound effect.
    PlayAudioParams params;
    params.audio = gAssetManager.LoadAudio("TEMPLEDEATHTAG.WAV", AssetScope::Scene);
    params.audioType = AudioType::Music;
    gAudioManager.Play(params);
}

void DeathScreen::Hide()
{
    // Pop layer off of stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);
}

void DeathScreen::OnUpdate(float deltaTime)
{
    RefreshUIScaling();
}

void DeathScreen::OnRetryButtonPressed()
{
    Hide();

    // If a scene would like to know when the "Retry" button is pressed, they can optionally specify a function in their Sheepscript called "PostDeath$".
    // For example, in the TE3 location, the function should be in the TE3.SHP file.
    std::string scriptStr = StringUtil::Format("wait CallSheep(\"%s\", \"PostDeath$\")", gLocationManager.GetLocation().c_str());
    gActionManager.ExecuteSheepAction(scriptStr);
}

void DeathScreen::RefreshUIScaling()
{
    Vector2 bgImageSize(640.0f, 480.0f);

    // The original game actually does scale this UI up to match the current resolution.
    // The logic is similar to the title screen, though the button logic differs.
    if(Prefs::UseOriginalUIScalingLogic() && Window::GetHeight() <= Prefs::GetMinimumScaleUIHeight())
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

    // This seems to give close to the same results of the original game for button positions.
    float buttonY = 24.0f + (bgImageSize.y - 480.0f) * 0.0917f;
    float distBetweenButtons = Math::RoundToInt(Math::Min(18.0f + (bgImageSize.x - 640.0f) * 0.15f, 100.0f));

    mRestoreButton->GetRectTransform()->SetAnchoredPosition(0.0f, buttonY);
    mRetryButton->GetRectTransform()->SetAnchoredPosition(-80 - distBetweenButtons, buttonY);
    mQuitButton->GetRectTransform()->SetAnchoredPosition(80 + distBetweenButtons, buttonY);
}