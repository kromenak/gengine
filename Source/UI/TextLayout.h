//
// Clark Kromenaker
//
// Given some text and layout constraints, is able to pre-calculate
// how the text will be laid out.
//
// This is helpful to understand pre-memory alloc how much text can fit
// in a space, or whether it will result in word-wrap, etc.
//
// There are probably a ton of ways this could be improved; a few ideas:
// 1) If we know ahead of time how many lines of text we will display, line y-pos calculations can be a lot more efficient.
// 2) Instead of splitting text using \n, we can just detect it and go to a new line as we parse the text.
//
#pragma once
#include <vector>

#include "Rect.h"
#include "Vector2.h"

class Font;
struct Glyph;

enum class HorizontalAlignment
{
	Left,
	Right,
	Center
};

enum class VerticalAlignment
{
	Bottom,
	Top,
	Center
};

enum class HorizontalOverflow
{
	Wrap,
	Overflow
};

enum class VerticalOverflow
{
	Truncate,
	Overflow
};

class TextLayout
{
public:
	struct CharInfo
	{
		CharInfo(Glyph& glyph, Vector2 pos) : glyph(glyph), pos(pos) { }
		CharInfo& operator=(const CharInfo& other);
		
        // Glyph to use when rendering this text character.
		Glyph& glyph;
        
        // Position of the text character (bottom-left corner).
		Vector2 pos;
	};

    static float GetLineHeight(Font* font, int lineNumber);
    static float GetTotalLineHeight(Font* font, int lineCount);
	
    TextLayout() = default;
	TextLayout(const Rect& rect, Font* font,
			   HorizontalAlignment ha, VerticalAlignment va,
			   HorizontalOverflow ho, VerticalOverflow vo);
	
	TextLayout(TextLayout& other) = default;
	TextLayout(TextLayout&& other) = default;
	TextLayout& operator=(const TextLayout& other) = default;

    // Lines
	void AddLine(const std::string& line);
	int GetLineCount() const { return mLineCount; }

    // Chars
    int GetCharCount() const { return (int)mCharInfos.size(); }
	const CharInfo* GetChar(int index) const;
	const std::vector<CharInfo>& GetChars() const { return mCharInfos; }
	Vector2 GetNextCharPos() const { return mNextCharPos; }

private:
	// The rect in which the text will be laid out.
	Rect mRect;
	
	// The font that will be used.
	Font* mFont = nullptr;
	
	// Horizontal and vertical alignment requirements.
	HorizontalAlignment mHorizontalAlignment = HorizontalAlignment::Left;
	VerticalAlignment mVerticalAlignment = VerticalAlignment::Top;
	
	// Horizontal and vertical overflow requirements.
	HorizontalOverflow mHorizontalOverflow = HorizontalOverflow::Overflow;
	VerticalOverflow mVerticalOverflow = VerticalOverflow::Overflow;
	
	// The total number of lines.
	int mLineCount = 0;

	// Info about each char that will be rendered.
	std::vector<CharInfo> mCharInfos;
	
	// The position that the next char would go, if we had one.
	Vector2 mNextCharPos;
};
