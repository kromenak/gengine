//
// Clark Kromenaker
//
// A scrollbar is a compound widget that consists of a slider, a drag handle, and two buttons.
// Scrollbars can be either vertical or horizontal (though this class is limited to vertical scrolling for now).
//
// The most common use case for a scrollbar is to navigate some UI content that is larger than its visible area.
// For example, controlling which portion of a scroll are is visible.
//
#pragma once
#include "UIWidget.h"

#include <functional>

#include "Color32.h"
#include "UINineSlice.h"

class Texture;
class UIButton;
class UIDrag;
class UISlider;

struct UIScrollbarParams
{
    // Textures for the button that decreases scroll value (typically up or left buttons).
    Texture* decreaseValueButtonUp = nullptr;
    Texture* decreaseValueButtonDown = nullptr;

    // Textures for the button that increases scroll value (typically down or right buttons).
    Texture* increaseValueButtonUp = nullptr;
    Texture* increaseValueButtonDown = nullptr;

    // A tileable texture for the scrollbar backing, along with a color tint.
    // The backing texture is optional - if not specified, just the color is used, and default width/height is derived from button textures.
    Texture* scrollbarBacking = nullptr;
    Color32 scrollbarBackingColor = Color32::White;

    // Params controlling the appearance of the scrollbar handle.
    UINineSliceParams handleParams;
};

class UIScrollbar : public UIWidget
{
public:
    UIScrollbar(Actor* owner, const UIScrollbarParams& params);

    void SetValue(float normalizedValue);
    void SetValueSilently(float normalizedValue);

    void SetHandleSize(float size);
    void SetHandleNormalizedSize(float size);
    bool IsHandleBeingDragged() const;

    void SetDecreaseValueCallback(const std::function<void()>& callback) { mDecreaseValueCallback = callback; }
    void SetIncreaseValueCallback(const std::function<void()>& callback) { mIncreaseValueCallback = callback; }
    void SetValueChangeCallback(const std::function<void(float)>& callback) { mValueChangeCallback = callback; }

    void SetCanInteract(bool canInteract);

private:
    // The slider used for the scrollbar drag behavior.
    UISlider* mSlider = nullptr;

    // The handle that is dragged within the slider.
    UIDrag* mHandle = nullptr;

    // The area that the handle is dragged within. Needed for size calculations.
    RectTransform* mHandleBacking = nullptr;

    // Buttons for decreasing/increasing scroll.
    UIButton* mDecreaseValueButton = nullptr;
    UIButton* mIncreaseValueButton = nullptr;

    // Callbacks that are fired when the buttons to decrease/increase scroll value are pressed.
    std::function<void()> mDecreaseValueCallback = nullptr;
    std::function<void()> mIncreaseValueCallback = nullptr;

    // Callback that's fired when the scroll value changes.
    std::function<void(float)> mValueChangeCallback = nullptr;

    void OnDecreaseValueButtonPressed();
    void OnIncreaseValueButtonPressed();
    void OnSliderValueChanged(float value);
};