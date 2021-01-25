//
// TextLayout.cpp
//
// Clark Kromenaker
//
#include "TextLayout.h"

#include "Font.h"
#include "StringUtil.h"

TextLayout::CharInfo& TextLayout::CharInfo::operator=(const TextLayout::CharInfo& other)
{
	glyph = other.glyph;
	pos = other.pos;
	return *this;
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
    // However, we may need to add a line break if line is too long and horizontal overflow mode is "wrap"!
    
    // Add one line.
    ++mLineCount;
    
    // Determine length of this line.
    //TODO: This logic DOES NOT currently account for horizontal wrapping correctly!
    int lineWidth = 0;
    for(size_t i = 0; i < line.size(); ++i)
    {
        Glyph& glyph = mFont->GetGlyph(line[i]);
        lineWidth += glyph.width;
    }
	
    // Height of the line is easier - always glyph height from the font.
	int lineHeight = mFont->GetGlyphHeight();
	
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
	//case VerticalAlignment::Center:
	//	break;
	}
	
    // OK, we know x/y to start the line at.
    // Determine CharInfo for each text character: the glyph and position of the glyph for rendering.
	for(size_t i = 0; i < line.size(); ++i)
	{
		Glyph& glyph = mFont->GetGlyph(line[i]);
		
		float leftX = xPos;
		float rightX = xPos + glyph.width;
		
        //TODO: This KIND OF works, but I think horizontal wrap detection should really occur earlier in this function
        //TODO: to avoid duplicate code AND get correct results!
		// If this glyph will extend outside the horizontal bounds of the rect, and we want to wrap, move to a new line!
        if(mHorizontalOverflow == HorizontalOverflow::Wrap &&
		   (leftX < mRect.GetMin().x || rightX > mRect.GetMax().x))
		{
            // Adding another line!
            ++mLineCount;
            
            // Determine new x-pos for this next line.
			switch(mHorizontalAlignment)
			{
			case HorizontalAlignment::Left:
				xPos = mRect.GetMin().x;
				break;
			case HorizontalAlignment::Right:
                //TODO: This doesn't seem correct - shouldn't it be "remainingLineWidth" or something?
				xPos = mRect.GetMax().x - lineWidth;
				break;
			case HorizontalAlignment::Center:
                std::cout << "CENTER HORIZONTAL ALIGNMENT NOT ACCOUNTED FOR!" << std::endl;
				break;
			}
			
            // Determine new y-pos for this next line.
			switch(mVerticalAlignment)
			{
			case VerticalAlignment::Bottom:
				yPos = mRect.GetMin().y;
                
                // If we add a new line with bottom alignment,
                // all previous characters must be moved up by one line!
				for(auto& charInfo : mCharInfos)
				{
					charInfo.pos.y += lineHeight;
				}
				break;
			case VerticalAlignment::Top:
                // Top: just move down "one line's worth".
				yPos += lineHeight;
				break;
			//case VerticalAlignment::Center:
			//	break;
			}
		}
		
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
