//
// UIImage.cpp
//
// Clark Kromenaker
//
#include "UIImage.h"

#include "Actor.h"
#include "Mesh.h"
#include "Texture.h"

extern Mesh* quad;

TYPE_DEF_CHILD(Component, UIImage);

UIImage::UIImage(Actor* owner) : UIWidget(owner)
{
    
}

void UIImage::Render()
{
	mMaterial.SetWorldTransformMatrix(GetUIWorldTransformMatrix());
	mMaterial.Activate();
	
	quad->Render();
}

void UIImage::SetTexture(Texture* texture)
{
	mMaterial.SetDiffuseTexture(texture);
	mSize.SetX(texture->GetWidth());
	mSize.SetY(texture->GetHeight());
}
