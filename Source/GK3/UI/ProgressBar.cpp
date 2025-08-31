#include "ProgressBar.h"

#include "AssetManager.h"
#include "Random.h"
#include "RectTransform.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"

ProgressBar::ProgressBar() : Actor("Progress Bar", TransformType::RectTransform),
    mLayer("ProgressBar")
{
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

    // Create canvas to contain the progress bar image.
    // Using a canvas here allows us to mask the progress bar image.
    mProgressBarCanvas = UI::CreateCanvas("BarImageCanvas", mBackground->GetOwner(), kCanvasOrder + 1);
    mProgressBarCanvas->SetMasked(true);
    mProgressBarCanvas->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mProgressBarCanvas->GetRectTransform()->SetAnchoredPosition(40.0f, 52.0f);
    mProgressBarCanvas->GetRectTransform()->SetSizeDelta(kProgressBarWidth, kProgressBarHeight);

    // Create bar image inside of canvas.
    // The pivot is important to get the correct progress bar effect when modifying the parent canvas's size.
    mProgressBarImage = UI::CreateWidgetActor<UIImage>("BarImage", mProgressBarCanvas->GetOwner());
    mProgressBarImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mProgressBarImage->SetTexture(gAssetManager.LoadTexture("PROGRESS_SLIDER.BMP"), true);
}

void ProgressBar::Show(Type type)
{
    // Change background based on the type of progress bar being used.
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

    // Each time the progress bar shows, the part of the bar image that's shown changes randomly.
    float randomY = Random::Range(0.0f, mProgressBarImage->GetTexture()->GetHeight() - kProgressBarHeight);
    mProgressBarImage->GetRectTransform()->SetAnchoredPosition(0.0f, -randomY);

    // Show the bar.
    SetActive(true);
    gLayerManager.PushLayer(&mLayer);

    // Make sure fake progress is disabled unless asked for.
    mFakeProgressDuration = 0.0f;
}

void ProgressBar::Hide()
{
    SetActive(false);

    // The progress bar is a bit unique in that other layers may be added to the layer stack after it.
    // For example, during a save game load, the scene layer will get added on top of this layer.
    if(gLayerManager.IsTopLayer(&mLayer))
    {
        gLayerManager.PopLayer(&mLayer);
    }
    else if(gLayerManager.IsLayerInStack(&mLayer))
    {
        gLayerManager.RemoveLayer(mLayer);
    }
}

void ProgressBar::SetProgress(float fraction)
{
    // The canvas is configured in such a way that we can just set the size (as a % of it's total width) and get the expected progress bar effect.
    mProgressBarCanvas->GetRectTransform()->SetSizeDeltaX(fraction * kProgressBarWidth);
}

void ProgressBar::ShowFakeProgress(float duration)
{
    mFakeProgressDuration = duration;
    mFakeProgressTimer = 0.0f;
}

void ProgressBar::OnUpdate(float deltaTime)
{
    // If a fake duration was specified....
    if(mFakeProgressDuration > 0.0f)
    {
        // And we haven't reached the duration yet...
        if(mFakeProgressTimer < mFakeProgressDuration)
        {
            // Increment timer and update progress bar.
            mFakeProgressTimer += deltaTime;
            SetProgress(mFakeProgressTimer / mFakeProgressDuration);

            // If we do reach the duration, hide the progress bar.
            if(mFakeProgressTimer >= mFakeProgressDuration)
            {
                Hide();
            }
        }
    }
}