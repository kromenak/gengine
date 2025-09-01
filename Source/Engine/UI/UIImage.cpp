#include "UIImage.h"

#include "Actor.h"
#include "Debug.h"
#include "Mesh.h"
#include "Texture.h"

extern Mesh* uiQuad;

//HACK: Need to revert quad UVs after rendering with repeat.
static float quad_uvs[] = {
    0.0f, 0.0f,     // upper-left
    1.0f, 0.0f,     // upper-right
    1.0f, 1.0f,     // lower-right
    0.0f, 1.0f      // lower-left
};

TYPEINFO_INIT(UIImage, UIWidget, 20)
{

}

UIImage::UIImage(Actor* owner) : UIWidget(owner)
{
    // Use white texture by default.
    SetTexture(&Texture::White);
}

void UIImage::Render()
{
    // Activate material and set world transform.
    mMaterial.Activate(GetWorldTransformWithSizeForRendering());

    // Render based on desired render mode.
    switch(mRenderMode)
    {
        case RenderMode::Normal:
        {
            uiQuad->Render();
            break;
        }
        case RenderMode::Tiled:
        {
            // We need a texture to render (and calculate repeats for tiled rendering).
            // If none is specified, use plain ol' white.
            Texture* texture = mMaterial.GetDiffuseTexture();
            if(texture == nullptr)
            {
                texture = &Texture::White;
            }

            // Determine how many repeats are needed.
            Vector2 size = GetRectTransform()->GetSize();
            float repeatX = size.x / texture->GetWidth();
            float repeatY = size.y / texture->GetHeight();

            // Adjust UVs for repeating textures.
            float repeat_uvs[] = {
                0.0f,    0.0f,		// upper-left
                repeatX, 0.0f,		// upper-right
                repeatX, repeatY,	// lower-right
                0.0f,    repeatY	// lower-left
            };
            uiQuad->GetSubmesh(0)->SetUV1s(repeat_uvs);

            // Render.
            uiQuad->Render();

            // Revert to "normal" UVs.
            uiQuad->GetSubmesh(0)->SetUV1s(quad_uvs);
        }
    }
}

void UIImage::SetColor(const Color32& color)
{
    mMaterial.SetColor(color);
}

void UIImage::SetTransparentColor(const Color32& color)
{
    mMaterial.SetColor("gDiscardColor", color);
}

void UIImage::SetTexture(Texture* texture, bool resizeImage)
{
    // Setting a null texture is actually an error (the system just renders garbage or whatever was leftover from last render).
    // So if no texture was set, go back to the default of white.
    if(texture != nullptr)
    {
        mMaterial.SetDiffuseTexture(texture);
    }
    else
    {
        mMaterial.SetDiffuseTexture(&Texture::White);
    }

    // If desired, resize widget rect to match texture.
    if(resizeImage)
    {
        ResizeToTexture();
    }
}

void UIImage::ResizeToTexture()
{
    // Need a texture to do this!
    Texture* texture = mMaterial.GetDiffuseTexture();
    if(texture == nullptr) { return; }

    // Set size from texture.
    GetRectTransform()->SetSizeDelta(texture->GetWidth(), texture->GetHeight());
}

void UIImage::ResizeToFitPreserveAspect(const Vector2& area)
{
    // Need a texture to do this!
    Texture* texture = mMaterial.GetDiffuseTexture();
    if(texture == nullptr) { return; }

    // Get width/height of texture.
    float width = texture->GetWidth();
    float height = texture->GetHeight();

    // Figure out new width/height for the texture if it filled the available area's width.
    float widthRatio = area.x / width;
    float newWidth = width * widthRatio;
    float newHeight = height * widthRatio;

    // However, if filling the width results in going offscreen vertically, we need to resize with that in mind instead!
    if(newHeight > area.y)
    {
        float heightRatio = area.y / newHeight;
        newWidth *= heightRatio;
        newHeight *= heightRatio;
    }

    // Set the size accordingly.
    GetRectTransform()->SetSizeDelta(newWidth, newHeight);
}

