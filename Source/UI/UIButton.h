//
// Clark Kromenaker
//
// A UI element that can be touched/clicked to execute some action.
//
#pragma once
#include "UIWidget.h"

#include <functional>

#include "Color32.h"
#include "Material.h"

class Audio;
class Texture;

class UIButton : public UIWidget
{
	TYPE_DECL_CHILD();
public:
	UIButton(Actor* actor);
	
	void Render() override;
    
    void SetUpTexture(Texture* texture, const Color32& color = Color32::White);
    void SetDownTexture(Texture* texture, const Color32& color = Color32::White);
    void SetHoverTexture(Texture* texture, const Color32& color = Color32::White);
    void SetDisabledTexture(Texture* texture, const Color32& color = Color32::White);

	void SetPressCallback(std::function<void(UIButton*)> callback) { mPressCallback = callback; }

    void SetHoverSound(Audio* sound) { mHoverSound = sound; }
	
	void OnPointerEnter() override;
	void OnPointerExit() override;
	
	void OnPointerDown() override;
	void OnPointerUp() override;
	
	void Press();
	
	bool CanInteract() const { return mCanInteract; }
	void SetCanInteract(bool canInteract) { mCanInteract = canInteract; }
	
private:
    // Defines what the button looks like in each state.
    struct State
    {
        Texture* texture = nullptr;
        Color32 color = Color32::Clear;
        bool IsSet() const { return texture != nullptr || color != Color32::Clear; }
    };
    State mUpState;
    State mDownState;
    State mHoverState;
    State mDisabledState;
	
	// Is the button interactive?
	// If not, it appears as a "disabled" button (e.g. grayed out) if a disabled texture is provided.
	bool mCanInteract = true;
	
	// Material used for rendering.
	Material mMaterial;
	
	// Callback to execute when the button is pressed.
	std::function<void(UIButton*)> mPressCallback;
	
	// Tracks pointer enter/exit and up/down for visual state and press checks.
	bool mPointerOver = false;
	bool mPointerDown = false;

    // Optional hover audio.
    Audio* mHoverSound = nullptr;
	
	Texture* GetDefaultTexture();
    void UpdateMaterial();
};
