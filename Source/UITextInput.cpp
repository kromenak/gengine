//
// UITextInput.cpp
//
// Clark Kromenaker
//
#include "UITextInput.h"

#include "Services.h"

TYPE_DEF_CHILD(UILabel, UITextInput);

UITextInput::UITextInput(Actor* owner) : UILabel(owner)
{
	
}

void UITextInput::Focus()
{
	if(!mFocused)
	{
		Services::GetInput()->StartListenForKeyboardInput(GetText());
	}
	mFocused = true;
}

void UITextInput::Unfocus()
{
	if(mFocused)
	{
		Services::GetInput()->StopListenForKeyboardInput();
	}
	mFocused = false;
}

void UITextInput::UpdateInternal(float deltaTime)
{
	// If mouse is down in the rect, let's assume that means we are focused.
	if(Services::GetInput()->IsMouseButtonDown(InputManager::MouseButton::Left))
	{
		bool focus = mRectTransform->GetScreenRect().Contains(Services::GetInput()->GetMousePosition());
		if(!mFocused && focus)
		{
			Focus();
		}
		if(mFocused && !focus)
		{
			Unfocus();
		}
	}
	
	// When focused, keep text up-to-date.
	if(mFocused)
	{
		SetText(Services::GetInput()->GetTextInput());
	}
	
	
}
