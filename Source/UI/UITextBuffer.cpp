#include "UITextBuffer.h"

#include <string>
#include <vector>

#include "Console.h"
#include "Font.h"

TYPE_DEF_CHILD(UILabel, UITextBuffer);

UITextBuffer::UITextBuffer(Actor* owner) : UILabel(owner)
{
	
}

float UITextBuffer::CalculateHeight() const
{
    return TextLayout::GetTotalLineHeight(GetFont(), mLineCount);
}

void UITextBuffer::OnUpdate(float deltaTime)
{
	auto& scrollback = gConsole.GetScrollback();
	if(mLastBufferLength != scrollback.size())
	{
		SetDirty();
	}
	mLastBufferLength = scrollback.size();
}

void UITextBuffer::PopulateTextLayout(TextLayout& textLayout)
{
    // The text buffer will display a subset of the console scrollback list.
	const std::vector<std::string>& scrollback = gConsole.GetScrollback();

    // The top of the text display is older text.
    // Subtract offset to get to the index of the last thing we want to show.
    int endIndex = Math::Max(0, scrollback.size() - 1 - mLineOffset);
    int startIndex = Math::Max(0, endIndex - mLineCount + 1);
    
	for(int i = startIndex; i <= endIndex; ++i)
	{
		// Stop if we run out of scrollback data.
		if(i >= scrollback.size()) { break; }
		
		// Add this line to the layout!
		textLayout.AddLine(scrollback[i]);
		
		// Scrollback will wrap lines that are too long, so a single line in the scrollback
		// might take up two or more in the actual text buffer, depending on resolution and rect settings.
		// So, check if line count meets desired line count and break out if so.
        //TODO: If the final line of the buffer wraps, we'll end up with more lines displayed than desired...need some way to fix that.
		if(textLayout.GetLineCount() >= mLineCount) { break; }
	}
}
