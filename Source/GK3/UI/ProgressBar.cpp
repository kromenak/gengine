#include "ProgressBar.h"

#include "AssetManager.h"
#include "RectTransform.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

ProgressBar::ProgressBar() : Actor("Progress Bar", TransformType::RectTransform),
    mLayer("ProgressBar")
{
    mLayer.OverrideAudioState(true);

    // Order should be pretty high, since this displays over almost everything.
    const int kCanvasOrder = 50;
    UI::AddCanvas(this, kCanvasOrder);

    // The background of the UI consists of a fullscreen clickable button area.
    // This stops interaction with whatever is below this UI.
    UIButton* button = AddComponent<UIButton>();
    button->SetPressCallback([](UIButton* button){});

    // Create background image. Default anchor properties (centered on screen) should be fine.
    mBackground = UI::CreateWidgetActor<UIImage>("Background", this);
    mBackground->SetTexture(gAssetManager.LoadTexture("PROGRESS_GENERIC.BMP"), true);

    // Create label for progress bar. Typically shows something like "Saving..." or "Restoring..."
    //UILabel* mLabel = UI::CreateWidgetActor<UILabel>(background->GetOwner());
    //mLabel->SetFont(gAssetManager.LoadFont("F_TEMPUS_A10.FON"));
    //mLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    //mLabel->SetVerticalAlignment(VerticalAlignment::Center);
    //mLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
    //mLabel->GetRectTransform()->SetSizeDelta(0.0f, 200.0f);

    // Create canvas to contain the progress bar image.
    // Using a canvas here allows us to mask the progress bar image.
    mProgressBarCanvas = UI::CreateCanvas("BarImageCanvas", mBackground->GetOwner(), kCanvasOrder + 1);
    mProgressBarCanvas->SetMasked(true);
    mProgressBarCanvas->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mProgressBarCanvas->GetRectTransform()->SetAnchoredPosition(40.0f, 52.0f);
    mProgressBarCanvas->GetRectTransform()->SetSizeDelta(513.0f, 50.0f);

    mProgressBarImage = UI::CreateWidgetActor<UIImage>("BarImage", mProgressBarCanvas->GetOwner());
    mProgressBarImage->SetTexture(gAssetManager.LoadTexture("PROGRESS_SLIDER.BMP"), true);
}

void ProgressBar::Show(Type type)
{
    switch(type)
    {
    case Type::Generic:
        mBackground->SetTexture(gAssetManager.LoadTexture("PROGRESS_GENERIC.BMP"), true);
        break;
    case Type::Save:
        mBackground->SetTexture(gAssetManager.LoadTexture("PROGRESS_SAVE_SCREEN.BMP"), true);
        break;
    case Type::Load:
        mBackground->SetTexture(gAssetManager.LoadTexture("PROGRESS_LOAD_SCREEN.BMP"), true);
        break;
    }
    SetActive(true);
    gLayerManager.PushLayer(&mLayer);
}

void ProgressBar::Hide()
{
    SetActive(false);
    gLayerManager.PopLayer(&mLayer);
}

void ProgressBar::SetProgress(float fraction)
{

}