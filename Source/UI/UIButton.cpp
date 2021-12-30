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
	// By default, a button ought to receive input.
    SetReceivesInput(true);
}

void UIButton::Render()
{
	if(!IsActiveAndEnabled()) { return; }
	
    // Update the texture to use.
    UpdateTexture();
	
	// Render.
    if(mMaterial.GetDiffuseTexture() != nullptr)
    {
        mMaterial.Activate(GetWorldTransformWithSizeForRendering());
        uiQuad->Render();
    }
}

void UIButton::SetUpTexture(Texture* texture)
{
    // Set texture and update.
    // It's important to call "UpdateTexture" here b/c it updates the dimensions of the widget.
    // If not called here, dimensions aren't updated until next render, which can cause layout issues for one frame.
    mUpTexture = texture;
    UpdateTexture();
}

void UIButton::SetDownTexture(Texture* texture)
{
    mDownTexture = texture;
    UpdateTexture();
}

void UIButton::SetHoverTexture(Texture* texture)
{
    mHoverTexture = texture;
    UpdateTexture();
}

void UIButton::SetDisabledTexture(Texture* texture)
{
    mDisabledTexture = texture;
    UpdateTexture();
}

void UIButton::OnPointerEnter()
{
    // If button has a texture, use cursor highlight.
    // If button has no texture (so, perhaps an input blocker or invisible click detector), no highlight.
    if(mMaterial.GetDiffuseTexture() != nullptr)
    {
        Services::Get<CursorManager>()->UseHighlightCursor();
    }
    else
    {
        Services::Get<CursorManager>()->UseDefaultCursor();
    }
	mPointerOver = true;
}

void UIButton::OnPointerExit()
{
    Services::Get<CursorManager>()->UseDefaultCursor();
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

void UIButton::UpdateTexture()
{
    // Figure out which texture we want to use.
    // Start by getting a default texture. If none exists, we can't render.
    Texture* texture = GetDefaultTexture();
    if(texture == nullptr) { return; }
    
    // If can interact, texture to use depends on up/down/hover texture availability and button state.
    // If can't interact, just use disabled texture if we have it!
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
    GetRectTransform()->SetSizeDelta(texture->GetWidth(), texture->GetHeight());
    
    // Set texture.
    mMaterial.SetDiffuseTexture(texture);
}
