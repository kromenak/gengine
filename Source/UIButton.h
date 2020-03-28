//
// UIButton.h
//
// Clark Kromenaker
//
// A UI element that can be touched/clicked to execute some action.
//
#pragma once
#include "UIWidget.h"

#include <functional>

#include "CallbackFunction.h"
#include "Material.h"

class Texture;

class UIButton : public UIWidget
{
	TYPE_DECL_CHILD();
public:
	UIButton(Actor* actor);
	
	void Render() override;
	
	void SetUpTexture(Texture* texture) { mUpTexture = texture; }
	void SetDownTexture(Texture* texture) { mDownTexture = texture; }
	void SetHoverTexture(Texture* texture) { mHoverTexture = texture; }
	void SetDisabledTexture(Texture* texture) { mDisabledTexture = texture; }
	
	void SetPressCallback(std::function<void()> callback) { mPressCallback = callback; }
	
	void OnPointerEnter() override;
	void OnPointerExit() override;
	
	void OnPointerDown() override;
	void OnPointerUp() override;
	
	void Press();
	
	bool CanInteract() const { return mCanInteract; }
	void SetCanInteract(bool canInteract) { mCanInteract = canInteract; }
	
protected:
	
private:
	// Textures for different visual states.
	// Up (normal), Down (pressed), Hover, and Disabled.
	Texture* mUpTexture = nullptr;
	Texture* mDownTexture = nullptr;
	Texture* mHoverTexture = nullptr;
	Texture* mDisabledTexture = nullptr;
	
	// Is the button interactive?
	// If not, it appears as a "disabled" button (e.g. grayed out) if a disabled texture is provided.
	bool mCanInteract = true;
	
	// Material used for rendering.
	Material mMaterial;
	
	// Callback to execute when the button is pressed.
	std::function<void()> mPressCallback;
	
	// Tracks pointer enter/exit and up/down for visual state and press checks.
	bool mPointerOver = false;
	bool mPointerDown = false;
	
	Texture* GetDefaultTexture();
};
