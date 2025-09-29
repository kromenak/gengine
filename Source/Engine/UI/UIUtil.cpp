#include "UIUtil.h"

#include "Actor.h"
#include "RectTransform.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "Window.h"

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

float UI::GetScaleFactor(float minimumUIScaleHeight, bool usePixelPerfectScaling, float bias)
{
    // Scale factor can be used by UIs to decide whether to scale up an asset that was authored for a lower resolution.
    // For example, a scale factor of 2 indicates that the asset should be rendered at 2x its authored size.

    // Only increase scale factor if above the minimum window height set globally.
    float scaleFactor = 1.0f;
    if(Window::GetHeight() >= minimumUIScaleHeight)
    {
        // Calculate how much to scale up based on the resolution.
        // GK3 UI was authored at 640x480 resolution - that's the lowest supported playable window size.
        // The amount to scale up is how much taller our resolution is than 480 pixels.
        constexpr float kReferenceHeight = 480.0f;
        scaleFactor = Math::Max(Window::GetHeight() / kReferenceHeight, 1.0f);

        // Whatever we calculated, the caller may want to nudge that value up or down in certain cases.
        // For example, maybe we're at 2x scale factor, but a particular UI looks better at one lower scale factor.
        scaleFactor = Math::Max(scaleFactor + bias, 1.0f);

        // To avoid artifacts from rendering UI images/glyphs across pixel boundaries, we only want integer scale factors.
        // This can be a bit limiting, but I haven't found another way to avoid artifacting yet.
        if(usePixelPerfectScaling)
        {
            scaleFactor = Math::Floor(scaleFactor);
        }
    }
    return scaleFactor;
}