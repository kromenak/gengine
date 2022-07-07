#include "SceneTransitioner.h"

#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"

SceneTransitioner::SceneTransitioner() : Actor(Actor::TransformType::RectTransform)
{
    // Draw scene transitioner above the normal scene.
    UICanvas* canvas = AddComponent<UICanvas>(0);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add overlay image that eats input.
    mOverlay = AddComponent<UIImage>();
    canvas->AddWidget(mOverlay);
    mOverlay->SetReceivesInput(true);

    // Overlay is see-through, but it tints the screen.
    mOverlay->SetTexture(&Texture::Black);
    mOverlay->SetColor(Color32(0, 0, 0, 128));
}

void SceneTransitioner::Show()
{
    SetActive(true);
}

void SceneTransitioner::Hide()
{
    SetActive(false);
}