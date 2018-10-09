//
// Font.h
//
// Clark Kromenaker
//
// A font that can be used to draw text in the game.
//
#pragma once
#include "Asset.h"

#include <unordered_map>

#include "Color32.h"
#include "Vector2.h"

class Texture;

struct Glyph
{
	// The character that is drawn with this glyph.
	char character = 'a';
	
	// Width and height of glyph, in pixels.
	int width;
	int height;
	
	// The UVs to use on a quad to render this glyph.
	Vector2 bottomLeftUvCoord;
	Vector2 topRightUvCoord;
	
	Vector2 topLeftUvCoord;
	Vector2 bottomRightUvCoord;
};

class Font : public Asset
{
public:
	Font(std::string name, char* data, int dataLength);
	~Font();
	
	Texture* GetTexture() const { return mFontTexture; }
	Glyph& GetGlyph(char character);
	
private:
	// A string containing all characters that can be rendered by this font.
	// Each character is in the same order it appears in the font texture.
	std::string mFontCharacters;
	
	// A character to use when the font can't render the desired character.
	// The default is usually "0x9D", which is graphically displayed as an "empty rectangle".
	char mDefaultChar = 0x9D;
	
	// The texture containing the font glyphs.
	// Blue dots in row 0 signifies the top-left corner of each glyph in the texture.
	// A single blue dot in column 0 signifies the "base" of each glyph.
	Texture* mFontTexture = nullptr;
	
	// When rendering, we can have extra space between characters or lines.
	int mExtraCharacterSpacing = 0;
	int mExtraLineSpacing = 0;
	
	int mLineCount = 1;
	
	// Background color (used for transparency).
	Color32 backgroundColor = Color32::Magenta;
	
	// A mapping from character to glyph.
	std::unordered_map<char, Glyph> mFontGlyphs;
	
	void ParseFromData(char* data, int dataLength);
};
