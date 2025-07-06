//
// Clark Kromenaker
//
// Similar to a UI button, but tapping it toggles an internal value on or off.
// Internally, this IS a button, but the "up" texture is changed based on the value being used.
//
#pragma once
#include "UIButton.h"

#include <functional>

class Texture;

class UIToggle : public UIButton
{
    TYPEINFO_SUB(UIToggle, UIButton);
public:
    UIToggle(Actor* owner);

    void SetOnTexture(Texture* texture);
    void SetOffTexture(Texture* texture);

    void SetValue(bool isOn);

    void SetToggleCallback(std::function<void(bool)> callback) { mToggleCallback = callback; }

private:
    // Toggle's value.
    bool mIsOn = false;

    // Textures to use for value on/off.
    Texture* mOnTexture = nullptr;
    Texture* mOffTexture = nullptr;

    // Callback to execute when toggle value changes.
    std::function<void(bool)> mToggleCallback;

    void OnPressed();
};
