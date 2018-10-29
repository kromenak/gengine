//
// UIButton.cpp
//
// Clark Kromenaker
//
#include "UIButton.h"

#include "Mesh.h"
#include "Services.h"
#include "Texture.h"

extern Mesh* quad;

TYPE_DEF_CHILD(UIWidget, UIButton);

UIButton::UIButton(Actor* owner) : UIWidget(owner)
{
	
}

void UIButton::Render()
{
	// Figure out which texture we want to use.
	// Start by getting a default texture. If none exists, we can't render.
	Texture* texture = GetDefaultTexture();
	if(texture == nullptr) { return; }
	
	// Split into enabled and disabled textures.
	if(mEnabled)
	{
		// Button is hovered if mouse position is within the screen rect for this widget.
		bool isHovered = GetScreenRect().Contains(Services::GetInput()->GetMousePosition());
		
		// Button is pressed if being hovered, and also the mouse is pressed down.
		bool isPressed = isHovered && Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Left);
		
		// This logic favors showing pressed, then hovered, then up states.
		if(isPressed && mDownTexture != nullptr)
		{
			texture = mDownTexture;
		}
		else if(isHovered && mHoverTexture != nullptr)
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
	mSize.SetX(texture->GetWidth());
	mSize.SetY(texture->GetHeight());
	
	// Set texture.
	mMaterial.SetDiffuseTexture(texture);
	
	// Set to correct location on screen.
	mMaterial.SetWorldTransformMatrix(GetUIWorldTransformMatrix());
	
	// Render.
	mMaterial.Activate();
	quad->Render();
}

Texture* UIButton::GetDefaultTexture()
{
	if(mUpTexture != nullptr) { return mUpTexture; }
	if(mHoverTexture != nullptr) { return mHoverTexture; }
	if(mDownTexture != nullptr) { return mDownTexture; }
	return mDisabledTexture;
}
