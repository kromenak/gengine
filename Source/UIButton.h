//
// UIButton.h
//
// Clark Kromenaker
//
// A UI element that can be touched/clicked to execute some action.
//
#pragma once
#include "UIWidget.h"

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
	
	void Enable() { mEnabled = true; }
	void Disable() { mEnabled = false; }
	
	void OnPointerEnter();
	void OnPointerExit();
	
	void OnPointerUp();
	void OnPointerDown();
	
private:
	// If enabled, the button can be interacted with.
	// If disabled, the button doesn't respond to inputs.
	bool mEnabled = true;
	
	CallbackFunction<UIButton>* mClickCallback = nullptr;
	
	// Textures for different visual states.
	// Up (normal), Down (pressed), Hover, and Disabled.
	Texture* mUpTexture = nullptr;
	Texture* mDownTexture = nullptr;
	Texture* mHoverTexture = nullptr;
	Texture* mDisabledTexture = nullptr;
	
	// Material used for rendering.
	Material mMaterial;
	
	Texture* GetDefaultTexture();
};
