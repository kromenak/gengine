//
// UIImage.cpp
//
// Clark Kromenaker
//
#include "UIImage.h"

#include "Actor.h"
#include "Mesh.h"
#include "Texture.h"

extern Mesh* axes;
extern Mesh* quad;

TYPE_DEF_CHILD(Component, UIImage);

UIImage::UIImage(Actor* owner) : UIWidget(owner)
{
    
}

void UIImage::Update(float deltaTime)
{
	//mOwner->SetPosition(mOwner->GetPosition() + (Vector3(128.0f, 128.0f, 0.0f) * deltaTime));
	//std::cout << mOwner->GetPosition() << std::endl;
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
