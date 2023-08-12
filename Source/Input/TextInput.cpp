#include "TextInput.h"

#include <iostream>

void TextInput::Insert(const std::string& text)
{
	Insert(text.c_str(), (int)text.size());
}

void TextInput::Insert(const char* text, int count)
{
	for(int i = 0; i < count; ++i)
	{
		Insert(text[i]);
	}
}

void TextInput::Insert(char c)
{
	// Make sure this isn't an exclude char.
	for(int i = 0; i < 4; ++i)
	{
		if(mExcludeChars[i] == c) { return; }
	}
	
	// Add the char to our text.
	if(mCursorPos < 0 || mCursorPos >= static_cast<int>(mText.size()))
	{
		mText.push_back(c);
	}
	else
	{
		mText.insert(mText.begin() + mCursorPos, c);
	}
	
	// Increment cursor pos, if set.
	if(mCursorPos >= 0)
	{
		++mCursorPos;
	}
}

void TextInput::DeletePrev()
{
	// This is like pressing "backspace" on your keyboard.
	
	// Can only delete if there's any text.
	if(mText.size() > 0)
	{
		// Out-of-range cursor pos means just delete from end.
		// Otherwise, delete one before the cursor pos.
		// If cursor pos is 0, this does nothing!
		if(mCursorPos < 0 || mCursorPos >= static_cast<int>(mText.size()))
		{
			mText.pop_back();
		}
		else if(mCursorPos != 0)
		{
			mText.erase(mCursorPos - 1, 1);
			
			// Cursor pos just decreased by one!
			--mCursorPos;
		}
	}
}

void TextInput::DeleteNext()
{
	// This is like pressing "delete" on your keyboard.
	
	// Can only delete if there's any text.
	if(mText.size() > 0)
	{
		// For a cursor at the end of the text, this would do nothing.
		// Otherwise, it deletes the current character.
		// Cursor pos does not change!
		if(mCursorPos >= 0 && mCursorPos < static_cast<int>(mText.size()))
		{
			mText.erase(mCursorPos, 1);
		}
	}
}

void TextInput::SetCursorPos(int pos)
{
	if(pos < 0 || pos >= static_cast<int>(mText.size()))
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
	if(mCursorPos >= 0 && mCursorPos < static_cast<int>(mText.size()))
	{
		++mCursorPos;
	}
	
	// If cursor went out-of-bounds, just reset to -1 (means 'end of text').
	if(mCursorPos >= static_cast<int>(mText.size()))
	{
		mCursorPos = -1;
	}
}

void TextInput::MoveCursorBack()
{
	// If cursor is out-of-bounds (meaning 'end of text'), just move back one.
	if(mCursorPos < 0 || mCursorPos >= static_cast<int>(mText.size()))
	{
		mCursorPos = (int)mText.size() - 1;
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

void TextInput::SetExcludeChar(int pos, char exclude)
{
	if(pos < 0 || pos >= 4) { return; }
	mExcludeChars[pos] = exclude;
}

void TextInput::SetText(const std::string& text)
{
	mText = text;
	
	// Reset cursor pos if it's now too large.
	if(mCursorPos >= static_cast<int>(mText.size()))
	{
		mCursorPos = -1;
	}
}
