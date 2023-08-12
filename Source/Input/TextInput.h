//
// Clark Kromenaker
//
// Provides operations for editing a piece of text with a keyboard.
//
#pragma once
#include <string>

class TextInput
{
public:
	void Insert(const std::string& text);
	void Insert(const char* text, int count);
	void Insert(char c);
	
	void DeletePrev();
	void DeleteNext();
	
	void SetCursorPos(int pos);
	int GetCursorPos() const { return mCursorPos; }
	void ClearCursorPos() { mCursorPos = -1; }
	
	void MoveCursorBack();
	void MoveCursorForward();
	void MoveCursorToStart();
	void MoveCursorToEnd();
	
	void SetExcludeChar(int pos, char exclude);
	
	void SetText(const std::string& text);
	const std::string& GetText() const { return mText; }
	
private:
	// The text that's being edited.
	std::string mText;
	
	// May want to exclude certain characters.
	// This approach is extremely simplistic, but it may have some speed advantages over like a set.
	// And we'll probably only ever need to exclude a couple chars in special situations.
	char mExcludeChars[4] { 0, 0, 0, 0 };
	
	// If set, the cursor is positioned before the given index.
	// E.g. 0 = cursor is before first character.
	// -1 indicates default behavior - cursor is at end.
	int mCursorPos = -1;
};
