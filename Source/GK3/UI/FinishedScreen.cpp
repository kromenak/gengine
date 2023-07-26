#include "FinishedScreen.h"

#include "AssetManager.h"
#include "GEngine.h"
#include "UICanvas.h"
#include "UIImage.h"

FinishedScreen::FinishedScreen() : Actor(TransformType::RectTransform)
{
    AddComponent<UICanvas>(1);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add finished screen background image.
    UIImage* background = AddComponent<UIImage>();
    background->SetTexture(gAssetManager.LoadTexture("FINISHED.BMP"));
    background->SetReceivesInput(true);
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
        GEngine::Instance()->Quit();
    }
}