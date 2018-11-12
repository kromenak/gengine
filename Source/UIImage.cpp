//
// UIImage.cpp
//
// Clark Kromenaker
//
#include "UIImage.h"

#include "Actor.h"
#include "CameraComponent.h"
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
	
	//Rect screenRect = mRectTransform->GetScreenRect();
	//Vector3 from = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMin(), 0.0f);
	//Vector3 to = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMax(), 0.0f);
	//Debug::DrawLine(from, to, Color32::Red);
}

void UIImage::SetTexture(Texture* texture)
{
	mMaterial.SetDiffuseTexture(texture);
	mRectTransform->SetSize(texture->GetWidth(), texture->GetHeight());
}
