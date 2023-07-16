#include "FinishedScreen.h"

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
    background->SetTexture(Services::GetAssets()->LoadTexture("FINISHED.BMP"));
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
    if(mWaitingForNoInput && Services::GetInput()->IsAnyKeyPressed())
    {
        return;
    }
    mWaitingForNoInput = false;

    // The finished screen stays up until any input is received while on the finished screen.
    if(Services::GetInput()->IsAnyKeyPressed() ||
       Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Left) ||
       Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Right) ||
       Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Middle))
    {
        GEngine::Instance()->Quit();
    }
}