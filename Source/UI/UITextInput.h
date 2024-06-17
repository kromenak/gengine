//
// Clark Kromenaker
//
// A text field allows the user to enter text.
//
#pragma once
#include "UILabel.h"

#include "TextInput.h"

class UITextInput : public UILabel
{
    TYPEINFO_SUB(UITextInput, UILabel);
public:
	UITextInput(Actor* owner);
	
	void Focus();
	void Unfocus();
	
	void Clear();
	
	void SetCaret(UIWidget* caret) { mCaret = caret; }
	void SetCaretBlinkInterval(float interval) { mCaretBlinkInterval = interval; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// Is the text input field focused?
	// If so, typing text will fill in the input field!
	bool mFocused = false;
	
	// Contains our text input buffer.
	TextInput mTextInput;
	
	// Optionally, a caret (i.e. text cursor) can be provided.
	UIWidget* mCaret = nullptr;
	float mCaretBlinkInterval = 0.0f;
	float mCaretBlickTimer = 0.0f;
};
