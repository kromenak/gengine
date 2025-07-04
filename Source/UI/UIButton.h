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
    TYPEINFO_SUB(UIButton, UIWidget);
public:
    UIButton(Actor* actor);

    void Render() override;

    void SetUpTexture(Texture* texture, const Color32& color = Color32::White);
    void SetDownTexture(Texture* texture, const Color32& color = Color32::White);
    void SetHoverTexture(Texture* texture, const Color32& color = Color32::White);
    void SetDisabledTexture(Texture* texture, const Color32& color = Color32::White);

    void SetPressCallback(const std::function<void(UIButton*)>& callback) { mPressCallback = callback; }

    void SetHoverSound(Audio* sound) { mHoverSound = sound; }
    bool IsHovered() const { return mPointerOver; }
    bool IsPressedDown() const { return sDownButton == this; }

    void OnPointerEnter() override;
    void OnPointerExit() override;

    void OnPointerDown() override;
    void OnPointerUp() override;

    void Press();
    void AnimatePress(float duration = 0.25f);

    bool CanInteract() const { return mCanInteract; }
    void SetCanInteract(bool canInteract) { mCanInteract = canInteract; }

    void SetResizeBasedOnTexture(bool resize) { mResizeBasedOnTexture = resize; }
    void SetHighlightCursorOnHover(bool highlight) { mForceCursorHighlightOnHover = highlight; }

    void SetTooltipText(const std::string& tooltipText) { mTooltipText = tooltipText; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // When a button is down, it gets some special treatment/logic. So, we need to keep track of it!
    static UIButton* sDownButton;

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

    // If true, button is resized based on texture used for the current state.
    // If false, we assume the button size is manually set, or driven by anchors and parenting.
    bool mResizeBasedOnTexture = true;

    // Material used for rendering.
    Material mMaterial;

    // Callback to execute when the button is pressed.
    std::function<void(UIButton*)> mPressCallback;

    // Tracks pointer enter/exit and up/down for visual state and press checks.
    bool mPointerOver = false;
    bool mPointerDown = false;

    // Optional hover audio.
    Audio* mHoverSound = nullptr;

    // If set, this button will ALWAYS show highlighted cursor when hovered, even if not image-based.
    bool mForceCursorHighlightOnHover = false;

    // An optional tooltip that is shown when this button is hovered.
    std::string mTooltipText;

    // If non-zero, the button should appear to be pressed to graphically show a programmatic press occurred.
    float mProgrammaticPressTimer = 0.0f;

    Texture* GetDefaultTexture();
    void UpdateMaterial();

    void ShowTooltip();
    void HideTooltip();
};
