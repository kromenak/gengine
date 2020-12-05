//
// UIImage.cpp
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
	TYPE_DECL_CHILD();
	
	enum class RenderMode
	{
		Normal,	// Stretches texture if image is bigger/smaller than texture.
		Tiled	// Keeps texture at normal size, but tiles it if image is bigger/smaller.
	};
	
public:
    UIImage(Actor* actor);
	
    void Render() override;
	
	void SetTexture(Texture* texture);
	void SetTextureAndSize(Texture* texture);
	
	void SetSizeToTextureSize();
	
	void SetColor(const Color32& color);
	
	void SetRenderMode(RenderMode mode) { mRenderMode = mode; }
	
private:
	Material mMaterial;
	RenderMode mRenderMode = RenderMode::Normal;
};
