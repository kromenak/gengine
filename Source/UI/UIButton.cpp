#include "UIButton.h"

#include "Actor.h"
#include "Camera.h"
#include "Debug.h"
#include "Mesh.h"
#include "Services.h"
#include "RectTransform.h"
#include "Texture.h"

extern Mesh* uiQuad;

UIButton* UIButton::sDownButton = nullptr;

TYPE_DEF_CHILD(UIWidget, UIButton);

UIButton::UIButton(Actor* owner) : UIWidget(owner)
{
	// By default, a button ought to receive input.
    SetReceivesInput(true);

    // Tell material to use white texture by default.
    // As a result, if a texture is never set (color-only button), it'll still work.
    mMaterial.SetDiffuseTexture(&Texture::White);
}

void UIButton::Render()
{
	if(!IsActiveAndEnabled()) { return; }
	
    // Update the texture to use.
    UpdateMaterial();

    mMaterial.Activate(GetWorldTransformWithSizeForRendering());
    uiQuad->Render();
}

void UIButton::SetUpTexture(Texture* texture, const Color32& color)
{
    mUpState.texture = texture;
    mUpState.color = color;

    // Set texture and update.
    // It's important to update material here b/c it updates the dimensions of the widget.
    // If not called here, dimensions aren't updated until next render, which can cause layout issues for one frame.
    UpdateMaterial();
}

void UIButton::SetDownTexture(Texture* texture, const Color32& color)
{
    mDownState.texture = texture;
    mDownState.color = color;
    UpdateMaterial();
}

void UIButton::SetHoverTexture(Texture* texture, const Color32& color)
{
    mHoverState.texture = texture;
    mHoverState.color = color;
    UpdateMaterial();
}

void UIButton::SetDisabledTexture(Texture* texture, const Color32& color)
{
    mDisabledState.texture = texture;
    mDisabledState.color = color;
    UpdateMaterial();
}

void UIButton::OnPointerEnter()
{
    // If pointer was pressed down on a button, and it wasn't this button, ignore any pointer enters.
    // The pressed down button becomes "focused"/exclusive (so to speak) until the pointer is released, allowing selecting any button again.
    if(sDownButton != nullptr && sDownButton != this) { return; }

    // Record pointer over.
    mPointerOver = true;
    
    // Only show interaction prompts (visual cursor change, audio) if we can actually interact with it.
    if(mCanInteract)
    {
        // If button has a texture, use cursor highlight.
        // If button has no texture (so, perhaps an input blocker or invisible click detector), no highlight.
        const Color32* color = mMaterial.GetColor("uColor");
        if(mMaterial.GetDiffuseTexture() != nullptr && color != nullptr && color->GetA() > 0)
        {
            Services::Get<CursorManager>()->UseHighlightCursor();
        }
        else
        {
            Services::Get<CursorManager>()->UseDefaultCursor();
        }

        // Play hover sound, if set.
        if(mHoverSound != nullptr)
        {
            Services::GetAudio()->PlaySFX(mHoverSound);
        }
    }
}

void UIButton::OnPointerExit()
{
    Services::Get<CursorManager>()->UseDefaultCursor();
	mPointerOver = false;
}

void UIButton::OnPointerDown()
{
	mPointerDown = true;

    // Got the pointer down event, so we must be the down button now.
    sDownButton = this;
}

void UIButton::OnPointerUp()
{
	// Pointer up after also receiving the pointer down event means...you pressed it!
	if(mPointerDown)
	{
		Press();
	}
	mPointerDown = false;

    // Pointer is up, so we are no longer down.
    sDownButton = nullptr;
}

void UIButton::Press()
{
	if(mPressCallback && IsEnabled() && CanInteract())
	{
		mPressCallback(this);
	}
}

Texture* UIButton::GetDefaultTexture()
{
	if(mUpState.texture != nullptr) { return mUpState.texture; }
	if(mHoverState.texture != nullptr) { return mHoverState.texture; }
	if(mDownState.texture != nullptr) { return mDownState.texture; }
	return mDisabledState.texture;
}

void UIButton::UpdateMaterial()
{
    // If marked as pointer down, but pointer was released, clear that flag.
    // This can happen if mouse is pressed down on this widget, but released outside of the widget.
    if(mPointerDown && !Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        mPointerDown = false;

        // Since pointer is up, there's no way there can be any down button right now.
        sDownButton = nullptr;
    }

    // Figure out which state to use.
    State& state = mDisabledState;
    if(mCanInteract)
    {
        // The button shows as down when pressed down, but only if pointer is also over the button.
        // You can press down on a button and then move the pointer all over before releasing it!
        if(mPointerDown && mPointerOver)
        {
            state = mDownState;
        }
        // When in pointer down state, but moving pointer all over, the button should still show as hovered.
        else if(mPointerOver || mPointerDown) 
        {
            state = mHoverState;
        }
        else
        {
            state = mUpState;
        }
    }

    // Try to find fallback state if this state is not set.
    if(!state.IsSet())
    {
        if(mUpState.IsSet())
        {
            state = mUpState;
        }
        else if(mHoverState.IsSet())
        {
            state = mHoverState;
        }
        else if(mDownState.IsSet())
        {
            state = mDownState;
        }
        else
        {
            state = mDisabledState;
        }
    }

    // Set color - easy enough.
    mMaterial.SetColor(state.color);

    // If we have a texture, use it!
    if(state.texture != nullptr)
    {
        // If our rect transform's anchors are equal, we'll assume we want the size of the rect to equal the texture size.
        // However, if NOT equal, then size is dictated by parent and anchors, so don't mess with it. (On the Map screen, for example).
        if(mResizeBasedOnTexture && GetRectTransform()->GetAnchorMin() == GetRectTransform()->GetAnchorMax())
        {
            GetRectTransform()->SetSizeDelta(state.texture->GetWidth(), state.texture->GetHeight());
        }
        
        // Set texture.
        mMaterial.SetDiffuseTexture(state.texture);
    }
}