#include "UIToggle.h"

TYPEINFO_INIT(UIToggle, UIButton, 29)
{

}

UIToggle::UIToggle(Actor* owner) : UIButton(owner)
{
    SetPressCallback([this](UIButton* button) {
        OnPressed();
    });
}

void UIToggle::SetOnTexture(Texture* texture)
{
    mOnTexture = texture;
    SetValue(mIsOn);
}

void UIToggle::SetOffTexture(Texture* texture)
{
    mOffTexture = texture;
    SetValue(mIsOn);
}

void UIToggle::SetValue(bool isOn)
{
    // Change value.
    mIsOn = isOn;

    // Update texture used.
    SetUpTexture(mIsOn ? mOnTexture : mOffTexture);
}

void UIToggle::OnPressed()
{
    SetValue(!mIsOn);

    if(mToggleCallback)
    {
        mToggleCallback(mIsOn);
    }
}
