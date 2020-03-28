//
// UIButton.cpp
//
// Clark Kromenaker
//
#include "UIButton.h"

#include "Actor.h"
#include "Camera.h"
#include "Debug.h"
#include "Mesh.h"
#include "Services.h"
#include "RectTransform.h"
#include "Texture.h"

extern Mesh* uiQuad;

TYPE_DEF_CHILD(UIWidget, UIButton);

UIButton::UIButton(Actor* owner) : UIWidget(owner)
{
	
}

void UIButton::Render()
{
	if(!IsActiveAndEnabled()) { return; }
	
	// Figure out which texture we want to use.
	// Start by getting a default texture. If none exists, we can't render.
	Texture* texture = GetDefaultTexture();
	if(texture == nullptr) { return; }
	
	// Split into enabled and disabled textures.
	if(mCanInteract)
	{
		// This logic favors showing pressed, then hovered, then up states.
		if(mPointerDown && mDownTexture != nullptr)
		{
			texture = mDownTexture;
		}
		else if(mPointerOver && mHoverTexture != nullptr)
		{
			texture = mHoverTexture;
		}
		else if(mUpTexture != nullptr)
		{
			texture = mUpTexture;
		}
	}
	else
	{
		if(mDisabledTexture != nullptr)
		{
			texture = mDisabledTexture;
		}
	}
	
	// Make sure widget size matches texture size.
	mRectTransform->SetSizeDelta(texture->GetWidth(), texture->GetHeight());
	
	// Set texture.
	mMaterial.SetDiffuseTexture(texture);
	
	// Set to correct location on screen.
	mMaterial.SetWorldTransformMatrix(GetWorldTransformWithSizeForRendering());
	 
	// Render.
	mMaterial.Activate();
	uiQuad->Render();
}

void UIButton::OnPointerEnter()
{
	GEngine::inst->UseHighlightCursor();
	mPointerOver = true;
}

void UIButton::OnPointerExit()
{
	GEngine::inst->UseDefaultCursor();
	mPointerOver = false;
}

void UIButton::OnPointerDown()
{
	mPointerDown = true;
}

void UIButton::OnPointerUp()
{
	// Pointer up after also receiving the pointer down event means...you pressed it!
	if(mPointerDown)
	{
		Press();
	}
	mPointerDown = false;
}

void UIButton::Press()
{
	if(mPressCallback && IsEnabled())
	{
		mPressCallback();
	}
}

Texture* UIButton::GetDefaultTexture()
{
	if(mUpTexture != nullptr) { return mUpTexture; }
	if(mHoverTexture != nullptr) { return mHoverTexture; }
	if(mDownTexture != nullptr) { return mDownTexture; }
	return mDisabledTexture;
}
