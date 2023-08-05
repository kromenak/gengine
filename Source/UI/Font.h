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
class Shader;

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
    Font(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    void Load(uint8_t* data, uint32_t dataLength);

	Texture* GetTexture() const { return mFontTexture; }
	Glyph& GetGlyph(char character);
	
	Color32 GetColor() const { return mColor; }
    Color32 GetReplaceColor() const { return mReplaceColor; }
	
	Shader* GetShader() const;
	
	int GetGlyphHeight() const { return mGlyphHeight; }
	
private:
	// A string containing all characters that can be rendered by this font.
	// Each character is in the same order it appears in the font texture.
	std::string mFontCharacters;
	
	// A character to use when the font can't render the desired character.
	// The default is usually "0x9D", which is graphically displayed as an "empty rectangle".
	unsigned char mDefaultChar = 0x9D;
	
	// The texture containing the font glyphs.
	// Blue dots in row 0 signifies the top-left corner of each glyph in the texture.
	// A single blue dot in column 0 signifies the "base" of each glyph.
	Texture* mFontTexture = nullptr;
	
	// The line count in the font texture.
	// Used to determine how many lines of glyphs exist in the texture.
	int mLineCount = 1;
	
	// When rendering, we can have extra space between characters or lines.
	int mExtraCharacterSpacing = 0;
	int mExtraLineSpacing = 0;
    
    // The color this font uses for the text.
    // For alpha-blended, the text is simply tinted by this color.
    // For color replacement, a single color in the font texture is replaced with this color (in shader).
	Color32 mColor = Color32::White;

    // If color mode is "ColorReplace," this specifies what color should be replaced.
    Color32 mReplaceColor = Color32::White;
    
    // For "Alpha Blend" type fonts, background color indicates the color that should be transparent.
    Color32 mBackgroundColor = Color32::Magenta;
    
    // Color mode used by this font.
    // Dictates shader used to mix color with font texture.
    enum class ColorMode : char
    {
        AlphaBlend,     // Multiply texel and color
        ColorReplace,   // Replace texel with color, using only alpha from texel
    };
    ColorMode mColorMode = ColorMode::AlphaBlend;
    
	// The max height of a glyph in the font.
	// We can use this to make some assumptions about line height.
	int mGlyphHeight = 0;
	
	// A mapping from character to glyph.
	std::unordered_map<char, Glyph> mFontGlyphs;
	
	void ParseFromData(uint8_t* data, uint32_t dataLength);
};
