#include "TextLayout.h"

#include "Font.h"
#include "StringUtil.h"

TextLayout::CharInfo& TextLayout::CharInfo::operator=(const TextLayout::CharInfo& other)
{
	glyph = other.glyph;
	pos = other.pos;
	return *this;
}

/*static*/ float TextLayout::GetLineHeight(Font* font, int lineNumber)
{
    // The first line's height is just the glyph height.
    // Second and on lines have one extra pixel for spacing.
    return lineNumber <= 1 ? font->GetGlyphHeight() : font->GetGlyphHeight() + 1;
}

/*static*/ float TextLayout::GetTotalLineHeight(Font* font, int lineCount)
{
    // Height is based on number of lines, with first line slightly shorter than the rest.
    float height = 0.0f;
    if(lineCount > 0)
    {
        height += GetLineHeight(font, 1);
    }
    if(lineCount > 1)
    {
        height += (lineCount - 1) * GetLineHeight(font, 2);
    }
    return height;
}

TextLayout::TextLayout(const Rect& rect, Font* font,
	HorizontalAlignment ha, VerticalAlignment va,
	HorizontalOverflow ho, VerticalOverflow vo) :
	mRect(rect),
	mFont(font),
	mHorizontalAlignment(ha),
	mVerticalAlignment(va),
	mHorizontalOverflow(ho),
	mVerticalOverflow(vo)
{
	
}

void TextLayout::AddLine(const std::string& line)
{
	// Handle receiving text that has line breaks in it by...splitting and calling recursively!
	if(line.find('\n') != std::string::npos)
	{
		std::vector<std::string> lines = StringUtil::Split(line, '\n');
		for(auto& childLine : lines)
		{
			AddLine(childLine);
		}
		return;
	}
    
    // OK, we have a line of text that has no line breaks in it.
    // We will be adding at least one line as a result.
    ++mLineCount;
    
    // Determine the length of this line.
    // This is made more complex b/c we may need to "wrap" the text if it extends beyond the width of our rect.
    int nextLineStartDueToWrap = -1;
    int actualLineLength = line.size();
    int lineWidth = 0;
    {
        int lastSpaceIndex = -1;
        int lastSpaceWidth = -1;
        for(size_t i = 0; i < line.size(); ++i)
        {
            // Get the glyph for this character.
            Glyph& glyph = mFont->GetGlyph(line[i]);

            // If wrapping, check for when adding a glyph causes the line width to exceed the rect width.
            // In that case, we will need to wrap to another line.
            if(mHorizontalOverflow == HorizontalOverflow::Wrap)
            {
                int newWidth = lineWidth + glyph.width;
                if(newWidth > mRect.width)
                {
                    // Try to wrap the text on a space to keep full words on one line.
                    // Worst case, if no space was present, just wrap on the current character.
                    if(lastSpaceIndex >= 0)
                    {
                        actualLineLength = lastSpaceIndex + 1;
                        nextLineStartDueToWrap = lastSpaceIndex + 1;
                        lineWidth = lastSpaceWidth;
                    }
                    else
                    {
                        actualLineLength = i;
                        nextLineStartDueToWrap = actualLineLength;
                    }
                    break;
                }
                lineWidth = newWidth;

                // Track whenever we see a space, as that may be where we need to "backtrack" to perform a line wrap.
                if(line[i] == ' ')
                {
                    lastSpaceIndex = i;
                    lastSpaceWidth = lineWidth;
                }
            }
            else
            {
                // No line wrap? Easy mode - just keep making the line longer.
                lineWidth += glyph.width;
            }
        }
    }

    // If we are artificially line wrapping, trim any extra spaces from the end (so they don't mess up text positioning).
    if(actualLineLength < line.size())
    {
        Glyph& glyph = mFont->GetGlyph(' ');
        while(actualLineLength > 0 && line[actualLineLength - 1] == ' ')
        {
            --actualLineLength;
            lineWidth -= glyph.width;
        }
    }
	
    // Get height of the line based on font and number of lines.
    int lineHeight = GetLineHeight(mFont, mLineCount);

    // Determine initial left x-pos of this line, depending on alignment.
	float xPos = 0.0f;
	switch(mHorizontalAlignment)
	{
	case HorizontalAlignment::Left:
        // Left: just use min.x.
		xPos = mRect.GetMin().x;
		break;
	case HorizontalAlignment::Right:
        // Right: start at max.x and subtract total line width.
		xPos = mRect.GetMax().x - lineWidth;
		break;
	case HorizontalAlignment::Center:
        // Center: start at rect center and subtract half line width.
        xPos = mRect.GetMin().x + (mRect.GetSize().x / 2) - (lineWidth / 2);
		break;
	}
	
    // Determine bottom y-pos of this line, depending on alignment.
    float yPos = 0.0f;
    switch(mVerticalAlignment)
    {
    case VerticalAlignment::Bottom:
        // Bottom: just use min.y
        yPos = mRect.GetMin().y;

        // If we add a new line with bottom alignment,
        // all previous characters must be moved up by one line!
        for(auto& charInfo : mCharInfos)
        {
            charInfo.pos.y = charInfo.pos.y + lineHeight;
        }
        break;
    case VerticalAlignment::Top:
        // Top: max.y is top-y, but we need bottom-y.
        // So, we must subtract line height, and also deal with how many previous lines already exist.
        yPos = mRect.GetMax().y - (lineHeight * mLineCount);
        break;
    case VerticalAlignment::Center:
        // Center: Get vertical center-point of the rect. But subtract half line height because we need bottom-y.
        yPos = mRect.GetMin().y + (mRect.GetSize().y / 2) - (lineHeight / 2);
        //TODO: this currently only works for single line labels! Need to factor in mLineCount too.
        break;
    }
	
    // OK, we know x/y to start the line at.
    // Determine CharInfo for each text character: the glyph and position of the glyph for rendering.
	for(size_t i = 0; i < actualLineLength; ++i)
	{
		Glyph& glyph = mFont->GetGlyph(line[i]);
		
		//float leftX = xPos;
		//float rightX = xPos + glyph.width;

		// Calc bottom and top.
		float bottomY = yPos;
		float topY = yPos + glyph.height;
		
		// If this char will extend outside the vertical bounds of the rect, and we don't want overflow, stop!
		if(mVerticalOverflow == VerticalOverflow::Truncate &&
		   (bottomY < mRect.GetMin().y || topY > mRect.GetMax().y))
		{
			break;
		}
		
        // OK, we will render this character - put it on.
		mCharInfos.emplace_back(glyph, Vector2(xPos, yPos));
        
        // Move forward x-pos by the glyph's width.
		xPos += glyph.width;
	}
		
	// Save whatever the next glyph pos would be.
	mNextCharPos = Vector2(xPos, yPos);

    // If we have an artificial wrap, do the next line with the substring after what we've already processed.
    if(nextLineStartDueToWrap >= 0)
    {
        AddLine(line.substr(nextLineStartDueToWrap));
    }
}

const TextLayout::CharInfo* TextLayout::GetChar(int index) const
{
	if(index >= 0 && index < static_cast<int>(mCharInfos.size()))
	{
		return &mCharInfos[index];
	}
	return nullptr;
}

/*
// Old code for calculating y-pos for a line IF you already know how many lines there will be.
// Probably a useful optimization if number of lines is known ahead of time.

float xPos = 0.0f;
switch(mHorizontalAlignment)
{
	case HorizontalAlignment::Left:
		xPos = rect.GetMin().x;
		break;
	case HorizontalAlignment::Right:
		xPos = rect.GetMax().x - lineWidths[lIndex];
		break;
	//case HorizontalAlignment::Center:
	//	break;
}

float yPos = 0.0f;
switch(mVerticalAlignment)
{
	case VerticalAlignment::Bottom:
		yPos = rect.GetMin().y + ((lineHeight + mLineSpacing) * (lines.size() - 1 - lIndex));
		break;
	case VerticalAlignment::Top:
		yPos = rect.GetMax().y - ((lineHeight + mLineSpacing) * (lIndex + 1));
		break;
	//case VerticalAlignment::Center:
	//	break;
}
 
// Calculate desired pixel height from points.
//float desiredHeight = (mFontSizePt / 72.0f) * 96.0f;
//float desiredWidth = (desiredPixelHeight / glyph.height) * glyph.width;
*/
