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

TextLayout::TextLayout()
{
	
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
	
	int lineHeight = mFont->GetGlyphHeight();
	int lineWidth = 0;
	for(int i = 0; i < line.size(); ++i)
	{
		Glyph& glyph = mFont->GetGlyph(line[i]);
		lineWidth += glyph.width;
	}
	
	float xPos = 0.0f;
	switch(mHorizontalAlignment)
	{
	case HorizontalAlignment::Left:
		xPos = mRect.GetMin().GetX();
		break;
	case HorizontalAlignment::Right:
		xPos = mRect.GetMax().GetX() - lineWidth;
		break;
	//case HorizontalAlignment::Center:
	//	break;
	}
	
	float yPos = mNextCharPos.GetY() + lineHeight;
	switch(mVerticalAlignment)
	{
	case VerticalAlignment::Bottom:
		yPos = mRect.GetMin().GetY();
		for(auto& charInfo : mCharInfos)
		{
			charInfo.pos.SetY(charInfo.pos.GetY() + lineHeight);
		}
		break;
	case VerticalAlignment::Top:
		//yPos = mRect.GetMax().GetY() - ((lineHeight + mLineSpacing) * (lIndex + 1));
		break;
	//case VerticalAlignment::Center:
	//	break;
	}
	
	// We are adding at least one line...
	++mLineCount;
	
	for(int i = 0; i < line.size(); ++i)
	{
		Glyph& glyph = mFont->GetGlyph(line[i]);
		
		float leftX = xPos;
		float rightX = xPos + glyph.width;
		
		// If this glyph will extend outside the horizontal bounds of the rect, and we want to wrap, move to a new line!
		if(mHorizontalOverflow == HorizontalOverflow::Wrap &&
		   (leftX < mRect.GetMin().GetX() || rightX > mRect.GetMax().GetX()))
		{
			switch(mHorizontalAlignment)
			{
			case HorizontalAlignment::Left:
				xPos = mRect.GetMin().GetX();
				break;
			case HorizontalAlignment::Right:
				xPos = mRect.GetMax().GetX() - lineWidth;
				break;
			//case HorizontalAlignment::Center:
			//	break;
			}
			
			switch(mVerticalAlignment)
			{
			case VerticalAlignment::Bottom:
				yPos = mRect.GetMin().GetY();
				for(auto& charInfo : mCharInfos)
				{
					charInfo.pos.SetY(charInfo.pos.GetY() + lineHeight);
				}
				break;
			case VerticalAlignment::Top:
				yPos += lineHeight;
				break;
			//case VerticalAlignment::Center:
			//	break;
			}
			
			// Adding another line!
			++mLineCount;
		}
		
		// Calc bottom and top.
		float bottomY = yPos;
		float topY = yPos + glyph.height;
		
		// If this char will extend outside the vertical bounds of the rect, and we don't want overflow, stop!
		if(mVerticalOverflow == VerticalOverflow::Truncate &&
		   (bottomY < mRect.GetMin().GetY() || topY > mRect.GetMax().GetY()))
		{
			break;
		}
		
		mCharInfos.emplace_back(glyph, Vector2(xPos, yPos));
		xPos += glyph.width;
	}
		
	// Save whatever the next glyph pos would be.
	mNextCharPos = Vector2(xPos, yPos);
}

const TextLayout::CharInfo* TextLayout::GetChar(int index) const
{
	if(index >= 0 && index < mCharInfos.size())
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
		xPos = rect.GetMin().GetX();
		break;
	case HorizontalAlignment::Right:
		xPos = rect.GetMax().GetX() - lineWidths[lIndex];
		break;
	//case HorizontalAlignment::Center:
	//	break;
}

float yPos = 0.0f;
switch(mVerticalAlignment)
{
	case VerticalAlignment::Bottom:
		yPos = rect.GetMin().GetY() + ((lineHeight + mLineSpacing) * (lines.size() - 1 - lIndex));
		break;
	case VerticalAlignment::Top:
		yPos = rect.GetMax().GetY() - ((lineHeight + mLineSpacing) * (lIndex + 1));
		break;
	//case VerticalAlignment::Center:
	//	break;
}
 
// Calculate desired pixel height from points.
//float desiredHeight = (mFontSizePt / 72.0f) * 96.0f;
//float desiredWidth = (desiredPixelHeight / glyph.height) * glyph.width;
*/
