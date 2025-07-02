#include "QuitPopup.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "GEngine.h"
#include "InputManager.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIUtil.h"

QuitPopup::QuitPopup() : Actor("QuitPopup", TransformType::RectTransform),
    mLayer("ConfirmQuitLayer")
{
    // When the quit popup shows, all underlying audio stops entirely.
    mLayer.OverrideAudioState(true);

    // Add a fullscreen canvas that tints whatever's under it.
    UI::AddCanvas(this, 40, Color32(0, 0, 0, 128));

    // Create background image.
    UIImage* popupBackground = UI::CreateWidgetActor<UIImage>("Background", this);
    popupBackground->SetTexture(gAssetManager.LoadTexture("QUITGAME.BMP"), true);

    // Create "yes" button.
    mYesButton = UI::CreateWidgetActor<UIButton>("YesButton", popupBackground);
    mYesButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mYesButton->GetRectTransform()->SetAnchoredPosition(261.0f, 59.0f);
    mYesButton->SetUpTexture(gAssetManager.LoadTexture("QG_YES_U.BMP"));
    mYesButton->SetHoverTexture(gAssetManager.LoadTexture("QG_YES_H.BMP"));
    mYesButton->SetDownTexture(gAssetManager.LoadTexture("QG_YES_D.BMP"));
    mYesButton->SetPressCallback([](UIButton* button){
        GEngine::Instance()->Quit();
        // No SFX for this button press, since the game is quitting.
    });

    // Create "no" button.
    mNoButton = UI::CreateWidgetActor<UIButton>("NoButton", popupBackground);
    mNoButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mNoButton->GetRectTransform()->SetAnchoredPosition(418.0f, 59.0f);
    mNoButton->SetUpTexture(gAssetManager.LoadTexture("QG_NO_U.BMP"));
    mNoButton->SetHoverTexture(gAssetManager.LoadTexture("QG_NO_H.BMP"));
    mNoButton->SetDownTexture(gAssetManager.LoadTexture("QG_NO_D.BMP"));
    mNoButton->SetPressCallback([this](UIButton* button){
        Hide();
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    });

    // Hide by default.
    SetActive(false);
}

void QuitPopup::Show()
{
    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);

    // If this appears during a non-interactive part of the game, force interactivity.
    // Without this, you can get soft-locked on this screen if you hit the window "X" button during a cutscene!
    GEngine::Instance()->SetAllowInteractDuringActions(true);
}

void QuitPopup::Hide()
{
    // Pop layer off stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);

    // Clear interact during actions flag.
    GEngine::Instance()->SetAllowInteractDuringActions(false);
}

void QuitPopup::OnUpdate(float deltaTime)
{
    // Keyboard shortcuts for yes/no buttons.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
    {
        mYesButton->AnimatePress();
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        mNoButton->AnimatePress();
    }
}
