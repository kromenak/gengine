//
// Clark Kromenaker
//
// A UI element that displays an image.
//
#pragma once
#include "UIWidget.h"

#include "Material.h"

class Color32;

class UIImage : public UIWidget
{
    TYPEINFO_SUB(UIImage, UIWidget);
public:
    enum class RenderMode
    {
        Normal,	// Stretches texture if image is bigger/smaller than texture.
        Tiled	// Keeps texture at normal size, but tiles it if image is bigger/smaller.
    };

    UIImage(Actor* actor);

    void Render() override;

    void SetColor(const Color32& color);
    void SetTransparentColor(const Color32& color);

    void SetTexture(Texture* texture, bool resizeImage = false);
    Texture* GetTexture() const { return mMaterial.GetDiffuseTexture(); }

    void SetRenderMode(RenderMode mode) { mRenderMode = mode; }

    void ResizeToTexture();
    void ResizeToFitPreserveAspect(const Vector2& area);

private:
    Material mMaterial;
    RenderMode mRenderMode = RenderMode::Normal;
};
