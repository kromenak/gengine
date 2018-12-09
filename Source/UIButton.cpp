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
		bool isHovered = mRectTransform->GetScreenRect().Contains(Services::GetInput()->GetMousePosition());
		
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
	mRectTransform->SetSize(texture->GetWidth(), texture->GetHeight());
	
	// Set texture.
	mMaterial.SetDiffuseTexture(texture);
	
	// Set to correct location on screen.
	mMaterial.SetWorldTransformMatrix(GetWorldTransformWithSizeForRendering());
	
	Rect screenRect = mRectTransform->GetScreenRect();
	Vector3 min = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMin(), 0.0f);
	Vector3 max = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMax(), 0.0f);
	//Rect worldRect(min, max);
	//Debug::DrawRect(worldRect, Color32::Red);
	Debug::DrawLine(min, max, Color32::Blue);
	
	// Render.
	mMaterial.Activate();
	quad->Render();
}

void UIButton::Press()
{
	if(mPressCallback)
	{
		mPressCallback();
	}
}

void UIButton::UpdateInternal(float deltaTime)
{
	// If left mouse button is pressed, record whether input began over this button.
	if(Services::GetInput()->IsMouseButtonDown(InputManager::MouseButton::Left))
	{
		mPointerBeganOver = mRectTransform->GetScreenRect().Contains(Services::GetInput()->GetMousePosition());
	}
	
	// If pointer began over this button, wait for left mouse button to be released.
	// If it is released over this button, that counts as a press.
	if(mPointerBeganOver
	   && Services::GetInput()->IsMouseButtonUp(InputManager::MouseButton::Left))
	{
		if(mRectTransform->GetScreenRect().Contains(Services::GetInput()->GetMousePosition()))
		{
			Press();
		}
		mPointerBeganOver = false;
	}
}

Texture* UIButton::GetDefaultTexture()
{
	if(mUpTexture != nullptr) { return mUpTexture; }
	if(mHoverTexture != nullptr) { return mHoverTexture; }
	if(mDownTexture != nullptr) { return mDownTexture; }
	return mDisabledTexture;
}
