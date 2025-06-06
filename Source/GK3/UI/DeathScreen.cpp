#include "DeathScreen.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "GK3UI.h"
#include "GEngine.h"
#include "LocationManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

DeathScreen::DeathScreen() : Actor("Death Screen", TransformType::RectTransform),
    mLayer("DeathLayer")
{
    mLayer.OverrideAudioState(true);

    // Canvas takes up the entire screen.
    UICanvas* canvas = AddComponent<UICanvas>(20);
    canvas->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    canvas->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);

    // Add a black background image.
    UIImage* blackBackground = AddComponent<UIImage>();
    blackBackground->SetTexture(&Texture::Black);

    // Add death screen background image.
    UIImage* background = UI::CreateWidgetActor<UIImage>("Background", this);
    background->SetTexture(gAssetManager.LoadTexture("DEATHSCREEN.BMP"), true);
    background->SetReceivesInput(true);

    // Add buttons for retry, replay, and quit.
    {
        UIButton* retryButton = UI::CreateWidgetActor<UIButton>("RetryButton", background);
        retryButton->SetUpTexture(gAssetManager.LoadTexture("DS_RTRY_N.BMP"));
        retryButton->SetHoverTexture(gAssetManager.LoadTexture("DS_RTRY_H.BMP"));
        retryButton->SetDownTexture(gAssetManager.LoadTexture("DS_RTRY_D.BMP"));
        retryButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_RTRY_X.BMP"));
        retryButton->SetPressCallback([this](UIButton* button){
            OnRetryButtonPressed();
        });
        retryButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        retryButton->GetRectTransform()->SetAnchoredPosition(181.0f, 24.0f);
    }
    {
        UIButton* restoreButton = UI::CreateWidgetActor<UIButton>("RestoreButton", background);
        restoreButton->SetUpTexture(gAssetManager.LoadTexture("DS_REST_N.BMP"));
        restoreButton->SetHoverTexture(gAssetManager.LoadTexture("DS_REST_H.BMP"));
        restoreButton->SetDownTexture(gAssetManager.LoadTexture("DS_REST_D.BMP"));
        restoreButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_REST_X.BMP"));
        restoreButton->SetPressCallback([](UIButton* button){
            gGK3UI.ShowLoadScreen();
        });
        restoreButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        restoreButton->GetRectTransform()->SetAnchoredPosition(280.0f, 24.0f);
    }
    {
        UIButton* quitButton = UI::CreateWidgetActor<UIButton>("QuitButton", background);
        quitButton->SetUpTexture(gAssetManager.LoadTexture("DS_QUIT_N.BMP"));
        quitButton->SetHoverTexture(gAssetManager.LoadTexture("DS_QUIT_H.BMP"));
        quitButton->SetDownTexture(gAssetManager.LoadTexture("DS_QUIT_D.BMP"));
        quitButton->SetDisabledTexture(gAssetManager.LoadTexture("DS_QUIT_X.BMP"));
        quitButton->SetPressCallback([](UIButton* button){
            //TODO: Should show "are you sure?" prompt.
            GEngine::Instance()->Quit();
        });
        quitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        quitButton->GetRectTransform()->SetAnchoredPosition(379.0f, 24.0f);
    }

    // Not active by default.
    SetActive(false);
}

void DeathScreen::Show()
{
    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);

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

void DeathScreen::OnRetryButtonPressed()
{
    Hide();

    // If a scene would like to know when the "Retry" button is pressed, they can optionally specify a function in their Sheepscript called "PostDeath$".
    // For example, in the TE3 location, the function should be in the TE3.SHP file.
    std::string scriptStr = StringUtil::Format("wait CallSheep(\"%s\", \"PostDeath$\")", gLocationManager.GetLocation().c_str());
    gActionManager.ExecuteSheepAction(scriptStr);
}
