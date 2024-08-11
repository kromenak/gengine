#include "UIUtil.h"

#include "Actor.h"
#include "RectTransform.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"

void UIUtil::AddCanvas(Actor* canvasActor, int canvasOrder)
{
    // Add canvas.
    canvasActor->AddComponent<UICanvas>(canvasOrder);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = canvasActor->GetComponent<RectTransform>();
    rectTransform->SetAnchor(AnchorPreset::CenterStretch);
    rectTransform->SetSizeDelta(0.0f, 0.0f);
}

void UIUtil::AddColorCanvas(Actor* canvasActor, int canvasOrder, const Color32& color)
{
    AddCanvas(canvasActor, canvasOrder);

    // Add background image that fills the entire canvas with desired color.
    UIImage* background = canvasActor->AddComponent<UIImage>();
    background->SetTexture(&Texture::White);
    background->SetColor(color);

    // Eat input so below UI or 3D scene don't react to inputs.
    background->SetReceivesInput(true);
}

UICanvas* UIUtil::NewUIActorWithCanvas(Actor* parent, int canvasOrder)
{
    Actor* uiActor = new Actor(TransformType::RectTransform);
    uiActor->GetTransform()->SetParent(parent->GetTransform());
    return uiActor->AddComponent<UICanvas>(canvasOrder);
}