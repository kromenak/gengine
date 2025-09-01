#include "UITextInput.h"

#include "Actor.h"
#include "InputManager.h"

TYPEINFO_INIT(UITextInput, UILabel, 28)
{

}

UITextInput::UITextInput(Actor* owner) : UILabel(owner)
{
    mTextInput.SetExcludeChar(0, '`');
}

void UITextInput::Render()
{
    // Make sure the caret is positioned correctly.
    // This requires generating the mesh beforehand, or else the text layout positions will be wrong.
    GenerateMesh();
    UpdateCaretPosition();

    // Render per usual.
    UILabel::Render();
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
        if(gInputManager.GetTextInput() == &mTextInput)
        {
            gInputManager.StopTextInput();
        }

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
}

void UITextInput::SetCaret(UIWidget* caret)
{
    // Save caret.
    mCaret = caret;
}

void UITextInput::OnDisable()
{
    UILabel::OnDisable();

    // Make sure caret is not still visible when this component is disabled.
    if(mCaret != nullptr)
    {
        mCaret->SetEnabled(false);
    }
}

void UITextInput::OnUpdate(float deltaTime)
{
    UILabel::OnUpdate(deltaTime);

    // If mouse is down in the rect, let's assume that means we are focused.
    if(mAllowInputToChangeFocus)
    {
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
    }

    // Check if still focused.
    // Only one text input can be focused at a time, and sometimes focus can be stolen unexpectedly.
    if(mFocused && gInputManager.GetTextInput() != &mTextInput)
    {
        mFocused = false;
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

        // Put caret in correct spot.
        UpdateCaretPosition();
    }
}

void UITextInput::UpdateCaretPosition()
{
    if(mCaret != nullptr)
    {
        // Get desired caret position, which is based on the cursor pos in the text input.
        // If no cursor pos, put at next character pos (end of text).
        Vector2 caretPos;
        if(mTextInput.GetCursorPos() == -1)
        {
            caretPos = GetNextCharPos();
        }
        else
        {
            caretPos = GetCharPos(mTextInput.GetCursorPos());
        }

        // Factor in the text input's pivot.
        caretPos += GetRectTransform()->GetPivot() * GetRectTransform()->GetSize();

        // Set the position.
        RectTransform* caretRT = mCaret->GetOwner()->GetComponent<RectTransform>();
        caretRT->SetAnchoredPosition(caretPos);
    }
}
