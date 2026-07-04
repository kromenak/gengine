#include "TextInput.h"

#include "TextEncode.h"

void TextInput::Insert(const std::string& insertText)
{
    // Get length of text in UTF-8 characters.
    int insertTextLength = utf8::distance(insertText.begin(), insertText.end());

    // If this is exactly one character (so, likely typed rather than copy/paste) and its an exclude character, ignore it!
    if(insertTextLength == 1)
    {
        for(uint32_t excludeCodePoint : mExcludeCodePoints)
        {
            if(Utf8::Utf8ToCodePoint(insertText) == excludeCodePoint)
            {
                return;
            }
        }
    }

    // Figure out how many characters from the passed in string to insert.
    // Only truncate if there's a max length and everything doesn't fit in the remaining space.
    int charactersToInsert = -1;
    if(mMaxLength >= 0)
    {
        int textLength = utf8::distance(mText.begin(), mText.end());
        int remainingCharacters = mMaxLength - textLength;
        if(remainingCharacters < insertTextLength)
        {
            charactersToInsert = remainingCharacters;
        }
    }

    // If cursor is at end of text, just append inserted text to the end.
    if(mCursorPos < 0)
    {
        if(charactersToInsert < 0)
        {
            mText.append(insertText);
        }
        else
        {
            mText.append(Utf8::Substring(insertText, 0, charactersToInsert));
        }
    }
    else // cursor is in middle of text
    {
        // Insert text at cursor position.
        if(charactersToInsert < 0)
        {
            Utf8::Insert(mText, mCursorPos, insertText);
            mCursorPos += insertTextLength;
        }
        else
        {
            Utf8::Insert(mText, mCursorPos, Utf8::Substring(insertText, 0, charactersToInsert));
            mCursorPos += charactersToInsert;
        }
    }
}

void TextInput::DeletePrev()
{
    // This is like pressing "backspace" on your keyboard.

    // Can only delete if there's any text.
    if(!mText.empty())
    {
        // Out-of-range cursor pos means just delete from end.
        // Otherwise, delete one before the cursor pos.
        // If cursor pos is 0, this does nothing!
        int textLength = utf8::distance(mText.begin(), mText.end());
        if(mCursorPos < 0 || mCursorPos >= textLength)
        {
            Utf8::PopBack(mText);
        }
        else if(mCursorPos != 0)
        {
            Utf8::Erase(mText, mCursorPos - 1, 1);

            // Cursor pos just decreased by one!
            --mCursorPos;
        }
    }
}

void TextInput::DeleteNext()
{
    // This is like pressing "delete" on your keyboard.

    // Can only delete if there's any text.
    if(!mText.empty())
    {
        // For a cursor at the end of the text, this would do nothing.
        // Otherwise, it deletes the current character.
        // Cursor pos does not change!
        int textLength = utf8::distance(mText.begin(), mText.end());
        if(mCursorPos >= 0 && mCursorPos < textLength)
        {
            Utf8::Erase(mText, mCursorPos, 1);

            // If cursor is now at end of the text, set cursor pos to -1 to signify this.
            if(mCursorPos == textLength - 1)
            {
                mCursorPos = -1;
            }
        }
    }
}

void TextInput::SetCursorPos(int pos)
{
    int textLength = utf8::distance(mText.begin(), mText.end());
    if(pos < 0 || pos >= textLength)
    {
        mCursorPos = -1;
    }
    else
    {
        mCursorPos = pos;
    }
}

void TextInput::MoveCursorForward()
{
    // Move cursor forward if in range.
    int textLength = utf8::distance(mText.begin(), mText.end());
    if(mCursorPos >= 0 && mCursorPos < textLength)
    {
        ++mCursorPos;
    }

    // If cursor went out-of-bounds, just reset to -1 (means 'end of text').
    if(mCursorPos >= textLength)
    {
        mCursorPos = -1;
    }
}

void TextInput::MoveCursorBack()
{
    // If cursor is out-of-bounds (meaning 'end of text'), just move back one.
    int textLength = utf8::distance(mText.begin(), mText.end());
    if(mCursorPos < 0 || mCursorPos >= textLength)
    {
        mCursorPos = textLength - 1;
    }
    else if(mCursorPos > 0)
    {
        --mCursorPos;
    }
}

void TextInput::MoveCursorToStart()
{
    mCursorPos = 0;
}

void TextInput::MoveCursorToEnd()
{
    mCursorPos = -1;
}

void TextInput::SetExcludeCodePoint(int pos, uint32_t codePoint)
{
    if(pos < 0 || pos >= 4) { return; }
    mExcludeCodePoints[pos] = codePoint;
}

void TextInput::SetText(const std::string& text)
{
    mText = text;

    // Reset cursor pos if it's now too large.
    int textLength = utf8::distance(mText.begin(), mText.end());
    if(mCursorPos >= textLength)
    {
        mCursorPos = -1;
    }
}
