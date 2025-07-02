#include "FinishedScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "InputManager.h"
#include "UIImage.h"
#include "UIUtil.h"

FinishedScreen::FinishedScreen() : Actor(TransformType::RectTransform)
{
    // Add canvas taking up entire screen with black background.
    UI::AddCanvas(this, 20, Color32::Black);

    // Add finished image.
    UIImage* background = UI::CreateWidgetActor<UIImage>("Image", this);
    background->SetTexture(gAssetManager.LoadTexture("FINISHED.BMP"), true);
}

void FinishedScreen::Show()
{
    SetActive(true);
    mWaitingForNoInput = true;
}

void FinishedScreen::OnUpdate(float deltaTime)
{
    // Make sure the player is not pushing any keys - ensure they have released any inputs from BEFORE entering this screen.
    if(mWaitingForNoInput && gInputManager.IsAnyKeyPressed())
    {
        return;
    }
    mWaitingForNoInput = false;

    // The finished screen stays up until any input is received while on the finished screen.
    if(gInputManager.IsAnyKeyPressed() ||
       gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left) ||
       gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Right) ||
       gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Middle))
    {
        // This one actually is a direct quit call - no quit popup in this instance.
        GEngine::Instance()->Quit();
    }
}