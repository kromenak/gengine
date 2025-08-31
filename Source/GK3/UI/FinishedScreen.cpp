#include "FinishedScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "GKPrefs.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

FinishedScreen::FinishedScreen() : Actor(TransformType::RectTransform)
{
    // Add canvas taking up entire screen with black background.
    mCanvas = UI::AddCanvas(this, 20, Color32::Black);

    // Add finished image.
    mBackgroundImage = UI::CreateWidgetActor<UIImage>("Image", this);
    mBackgroundImage->SetTexture(gAssetManager.LoadTexture("FINISHED.BMP"), true);
}

void FinishedScreen::Show()
{
    SetActive(true);
    mWaitForNoInput.Reset();
    RefreshUIScaling();
}

void FinishedScreen::OnUpdate(float deltaTime)
{
    // The finished screen stays up until any input is received while on the finished screen.
    if(mWaitForNoInput.IsAnyKeyLeadingEdge() || mWaitForNoInput.IsAnyMouseButtonLeadingEdge())
    {
        // This one actually is a direct quit call - no quit popup in this instance.
        GEngine::Instance()->Quit();
    }
}

void FinishedScreen::RefreshUIScaling()
{
    // The original game actually does scale this UI up to match the current resolution.
    // The logic is similar to the title screen, though this screen's a lot simpler because it has no buttons to position.
    if(Prefs::UseOriginalUIScalingLogic())
    {
        // Turn off canvas autoscaling. This sets canvas scale to 1, and width/height equal to window width/height.
        mCanvas->SetAutoScale(false);

        // Resize background image to fit within window size, preserving aspect ratio.
        mBackgroundImage->ResizeToFitPreserveAspect(Window::GetSize());
    }
    else // not using original game's logic.
    {
        // In this case, just use 640x480 and have it auto-scale when the resolution gets too big.
        mCanvas->SetAutoScale(true);
        mBackgroundImage->ResizeToTexture();
    }
}