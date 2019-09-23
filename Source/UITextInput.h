//
// UITextInput.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include "UILabel.h"

class UITextInput : public UILabel
{
	TYPE_DECL_CHILD();
public:
	UITextInput(Actor* owner);
	
	void Focus();
	void Unfocus();
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// Is the text input field focused?
	// If so, typing text will fill in the input field!
	bool mFocused = false;
};
