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
	
	void Enable() { mEnabled = true; }
	void Disable() { mEnabled = false; }
	
	void SetPressCallback(std::function<void()> callback) { mPressCallback = callback; }
	
	//void OnPointerEnter();
	//void OnPointerExit();
	
	//void OnPointerUp();
	//void OnPointerDown();
	
	void Press();
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// If enabled, the button can be interacted with.
	// If disabled, the button doesn't respond to inputs.
	bool mEnabled = true;
	
	// Textures for different visual states.
	// Up (normal), Down (pressed), Hover, and Disabled.
	Texture* mUpTexture = nullptr;
	Texture* mDownTexture = nullptr;
	Texture* mHoverTexture = nullptr;
	Texture* mDisabledTexture = nullptr;
	
	// Material used for rendering.
	Material mMaterial;
	
	std::function<void()> mPressCallback;
	
	bool mPointerBeganOver = false;
	
	Texture* GetDefaultTexture();
};
