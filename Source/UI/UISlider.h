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

class UISlider : public UIWidget
{
    TYPEINFO(UISlider, UIWidget);
public:
    UISlider(Actor* owner);

    void Render() override;

    void SetHandleActor(Actor* handle);

    void SetValueChangeCallback(std::function<void(float)> callback) { mValueChangeCallback = callback; }

    void SetValue(float value);
    void SetValueSilently(float value);
    float GetValue() const;

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Draggable object that represents the handle.
    UIDrag* mHandle = nullptr;

    // The *normalized* value of the slider.
    float mValue = 1.0f;

    // Called when the value of the slider changes.
    std::function<void(float)> mValueChangeCallback;

    void SetSliderFromValue();
    void SetValueFromSlider();
};