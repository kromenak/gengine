#include "UIUtil.h"

#include "Actor.h"
#include "RectTransform.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"

UICanvas* UI::AddCanvas(Actor* actor, int canvasOrder, const Color32& color)
{
    // Add canvas.
    UICanvas* canvas = actor->AddComponent<UICanvas>(canvasOrder);

    // Root canvases with no parents will be scaled using the auto-scaling functionality in the UICanvas class.
    // For non-root canvases, we need some sane default - let's assume they take up all the space of their parent.
    // This is what we want in a lot of cases. The caller can of course change this after the canvas is returned.
    if(actor->GetParent() != nullptr)
    {
        canvas->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        canvas->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
    }

    // If a color is needed, add an image with the desired color.
    if(color != Color32::Clear)
    {
        // Add background image that fills the entire canvas with desired color.
        UIImage* background = actor->AddComponent<UIImage>();
        background->SetTexture(&Texture::White);
        background->SetColor(color);

        // Eat input so below UI or 3D scene don't react to inputs.
        background->SetReceivesInput(true);
    }
    return canvas;
}

UICanvas* UI::CreateCanvas(const std::string& name, Actor* parent, int canvasOrder, const Color32& color)
{
    Actor* uiActor = new Actor(name, TransformType::RectTransform);
    if(parent != nullptr)
    {
        uiActor->GetTransform()->SetParent(parent->GetTransform());
    }
    return AddCanvas(uiActor, canvasOrder, color);
}

UICanvas* UI::CreateCanvas(const std::string& name, Component* parent, int canvasOrder, const Color32& color)
{
    return CreateCanvas(name, parent != nullptr ? parent->GetOwner() : nullptr, canvasOrder, color);
}
