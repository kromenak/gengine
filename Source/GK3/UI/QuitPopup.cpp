#include "QuitPopup.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "GEngine.h"
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
    UIButton* yesButton = UI::CreateWidgetActor<UIButton>("YesButton", popupBackground);
    yesButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    yesButton->GetRectTransform()->SetAnchoredPosition(261.0f, 59.0f);
    yesButton->SetUpTexture(gAssetManager.LoadTexture("QG_YES_U.BMP"));
    yesButton->SetHoverTexture(gAssetManager.LoadTexture("QG_YES_H.BMP"));
    yesButton->SetDownTexture(gAssetManager.LoadTexture("QG_YES_D.BMP"));
    yesButton->SetPressCallback([](UIButton* button){
        GEngine::Instance()->Quit();
        // No SFX for this button press, since the game is quitting.
    });

    // Create "no" button.
    UIButton* noButton = UI::CreateWidgetActor<UIButton>("NoButton", popupBackground);
    noButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    noButton->GetRectTransform()->SetAnchoredPosition(418.0f, 59.0f);
    noButton->SetUpTexture(gAssetManager.LoadTexture("QG_NO_U.BMP"));
    noButton->SetHoverTexture(gAssetManager.LoadTexture("QG_NO_H.BMP"));
    noButton->SetDownTexture(gAssetManager.LoadTexture("QG_NO_D.BMP"));
    noButton->SetPressCallback([this](UIButton* button){
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
}

void QuitPopup::Hide()
{
    // Pop layer off stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);
}