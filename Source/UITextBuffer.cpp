//
// UITextBuffer.cpp
//
// Clark Kromenaker
//
#include "UITextBuffer.h"

#include <string>
#include <vector>

#include "Services.h"

TYPE_DEF_CHILD(UILabel, UITextBuffer);

UITextBuffer::UITextBuffer(Actor* owner) : UILabel(owner)
{
	
}

void UITextBuffer::OnUpdate(float deltaTime)
{
	auto& scrollback = Services::GetConsole()->GetScrollback();
	if(mLastBufferLength != scrollback.size())
	{
		SetDirty();
	}
	mLastBufferLength = (int)scrollback.size();
}

void UITextBuffer::PopulateTextLayout(TextLayout& textLayout)
{
	auto& scrollback = Services::GetConsole()->GetScrollback();
	
	int startIndex = (int)scrollback.size() - mLineOffset - mLineCount;
	if(startIndex < 0)
	{
		startIndex = 0;
	}
	
	int endIndex = startIndex + mLineCount;
	for(int i = startIndex; i < endIndex; ++i)
	{
		// Stop if we run out of scrollback data.
		if(i >= scrollback.size()) { break; }
		
		// Add this line to the layout!
		textLayout.AddLine(scrollback[i]);
		
		// Scrollback will wrap lines that are too long, so a single line in the scrollback
		// might take up two or more in the actual text buffer, depending on resolution and rect settings.
		// So, check if line count meets desired line count and break out if so.
		if(textLayout.GetLineCount() >= mLineCount) { break; }
	}
}
