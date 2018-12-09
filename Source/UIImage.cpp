//
// UIImage.cpp
//
// Clark Kromenaker
//
#include "UIImage.h"

#include "Actor.h"
#include "Camera.h"
#include "Debug.h"
#include "Mesh.h"
#include "Texture.h"

extern Mesh* quad;

TYPE_DEF_CHILD(UIWidget, UIImage);

UIImage::UIImage(Actor* owner) : UIWidget(owner)
{
    
}

void UIImage::Render()
{
	mMaterial.SetWorldTransformMatrix(GetWorldTransformWithSizeForRendering());
	mMaterial.Activate();
	
	quad->Render();
}

void UIImage::SetTexture(Texture* texture)
{
	mMaterial.SetDiffuseTexture(texture);
	mRectTransform->SetSize(texture->GetWidth(), texture->GetHeight());
}
