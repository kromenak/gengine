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

class UIImage : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIImage(Actor* actor);
	
    void Render() override;
	
	void SetTexture(Texture* texture);
	
private:
	Material mMaterial;
};
