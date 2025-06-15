#include "PauseScreen.h"

#include "AssetManager.h"
#include "InputManager.h"
#include "UIImage.h"
#include "UIUtil.h"

PauseScreen::PauseScreen() : Actor("PauseScreen", TransformType::RectTransform),
    mLayer("PauseLayer")
{
    // When the pause screen shows, all underlying audio stops entirely.
    mLayer.OverrideAudioState(true);

    // Add a fullscreen canvas that tints whatever's under it.
    UI::AddCanvas(this, 39, Color32(0, 0, 0, 128));

    // Create background image.
    UIImage* background = UI::CreateWidgetActor<UIImage>("Paused", this);
    background->SetTexture(gAssetManager.LoadTexture("PAUSED.BMP"), true);

    // Hide by default.
    SetActive(false);
}

void PauseScreen::Show()
{
    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);

    mInputDelayTimer = 0.1f;
}

void PauseScreen::Hide()
{
    // Pop layer off stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);
}

void PauseScreen::OnUpdate(float deltaTime)
{
    // Don't allow dismissing until a bit of time has passed.
    if(mInputDelayTimer > 0.0f)
    {
        mInputDelayTimer -= deltaTime;
        if(mInputDelayTimer > 0.0f)
        {
            return;
        }
    }

    // As far as I know, the only way to close the pause screen is to hit the "P" key.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_P))
    {
        Hide();
    }
}