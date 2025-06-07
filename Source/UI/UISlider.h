//
// Clark Kromenaker
//
// A UI element that can be clicked and dragged to change a numeric value.
//
#pragma once
#include "UIWidget.h"

#include <functional>

class Texture;
class UIDrag;

enum class SliderOrientation
{
    LeftRight,
    RightLeft,
    BottomUp,
    TopDown
};

class UISlider : public UIWidget
{
    TYPEINFO_SUB(UISlider, UIWidget);
public:
    UISlider(Actor* owner);

    void SetOrientation(SliderOrientation orientation);
    void SetHandleActor(Actor* handle);

    void SetValueChangeCallback(const std::function<void(float)>& callback) { mValueChangeCallback = callback; }

    void SetValue(float value);
    void SetValueSilently(float value);
    float GetValue() const { return mValue; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The orientation of the slider.
    SliderOrientation mOrientation = SliderOrientation::LeftRight;

    // Draggable object that represents the handle.
    UIDrag* mHandle = nullptr;

    // The *normalized* value of the slider.
    float mValue = 0.0f;

    // Called when the value of the slider changes.
    std::function<void(float)> mValueChangeCallback = nullptr;

    void SetHandleFromValue();
    void SetValueFromHandle();
};