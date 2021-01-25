//
// UIToggle.cpp
//
// Clark Kromenaker
//
#include "UIToggle.h"

TYPE_DEF_CHILD(UIButton, UIToggle);

UIToggle::UIToggle(Actor* owner) : UIButton(owner)
{
    SetPressCallback(std::bind(&UIToggle::OnPressed, this));
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
