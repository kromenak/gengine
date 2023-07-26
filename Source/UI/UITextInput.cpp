#include "UITextInput.h"

#include "Actor.h"

TYPE_DEF_CHILD(UILabel, UITextInput);

UITextInput::UITextInput(Actor* owner) : UILabel(owner)
{
	mTextInput.SetExcludeChar(0, '`');
}

void UITextInput::Focus()
{
	// If we are going from unfocused to focused...
	if(!mFocused)
	{
		// Start capturing keyboard input with current input contents as starter value.
		mTextInput.SetText(GetText());
		gInputManager.StartTextInput(&mTextInput);
		
		// Reset caret blink info.
		mCaretBlickTimer = mCaretBlinkInterval;
		if(mCaret != nullptr)
		{
			mCaret->SetEnabled(true);
		}
	}
	
	// We're now focused!
	mFocused = true;
}

void UITextInput::Unfocus()
{
	// If going from focused to unfocused...
	if(mFocused)
	{
		// Stop capturing keyboard input.
		gInputManager.StopTextInput();
		
		// Turn off any caret.
		if(mCaret != nullptr)
		{
			mCaret->SetEnabled(false);
		}
	}
	
	// No longer focused!
	mFocused = false;
}

void UITextInput::Clear()
{
	mTextInput.SetText("");
	SetText("");
	//TODO: Reset caret position
}

void UITextInput::OnUpdate(float deltaTime)
{
	// If mouse is down in the rect, let's assume that means we are focused.
	if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
	{
		bool focus = GetRectTransform()->GetWorldRect().Contains(gInputManager.GetMousePosition());
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
		SetText(mTextInput.GetText());
	}
	
	// Update caret based on focus and blink state.
	if(mCaret != nullptr)
	{
		// When focused, the caret is either visible or blinking.
		// When not focused, the caret is not visible.
		if(mFocused)
		{
			// If a blink interval was set, the caret should blink.
			// Otherwise, we assume "no blink" - always on.
			if(mCaretBlinkInterval > 0.0f)
			{
				mCaretBlickTimer -= deltaTime;
				if(mCaretBlickTimer <= 0.0f)
				{
					mCaret->SetEnabled(!mCaret->IsEnabled());
					mCaretBlickTimer = mCaretBlinkInterval;
				}
			}
			else
			{
				mCaret->SetEnabled(true);
			}
		}
		else
		{
			mCaret->SetEnabled(false);
		}
		
		// Position caret right after last letter in the text.
		RectTransform* caretRT = mCaret->GetOwner()->GetComponent<RectTransform>();
		if(mTextInput.GetCursorPos() == -1)
		{
			caretRT->SetAnchoredPosition(GetNextCharPos());
		}
		else
		{
			caretRT->SetAnchoredPosition(GetCharPos(mTextInput.GetCursorPos()));
		}
	}
}
