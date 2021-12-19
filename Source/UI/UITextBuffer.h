//
// Clark Kromenaker
//
// An area that renders text. Similar to a UILabel, but it pulls
// its text from some external "buffer."
//
// Initial/primary use is implementing a scrollback buffer for an in-game console UI.
//
#pragma once
#include "UILabel.h"

class UITextBuffer : public UILabel
{
	TYPE_DECL_CHILD();
public:
	UITextBuffer(Actor* owner);
	
	void SetLineCount(int lineCount) { mLineCount = lineCount; SetDirty(); }
	void SetLineOffset(int lineOffset) { mLineOffset = lineOffset; SetDirty(); }
	
protected:
	void OnUpdate(float deltaTime) override;
	
	void PopulateTextLayout(TextLayout& textLayout) override;
	
private:
	// Number of lines to display in the buffer.
	int mLineCount = 10;
	
	// Offset from the end of the buffer to display.
	int mLineOffset = 0;
	
	// Last recorded buffer length; to determine whether we need to update the layout!
	int mLastBufferLength = 0;
};
