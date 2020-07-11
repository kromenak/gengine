//
// Font.cpp
//
// Clark Kromenaker
//
#include "Font.h"

#include "Color32.h"
#include "IniParser.h"
#include "Material.h"
#include "Services.h"
#include "StringUtil.h"

Font::Font(std::string name, char* data, int dataLength) :
	Asset(name)
{
	ParseFromData(data, dataLength);
	
	// After parsing, if we have no font texture, we can't do much more.
	if(mFontTexture == nullptr) { return; }
	
	// The pixel color at (1, 0) seems to always be glyph indicator color.
	Color32 glyphStartColor = mFontTexture->GetPixelColor32(1, 0);
	
	// The line height is the font texture height divided by number of lines.
	// The glyphs height is then that number, minus one (for the indicator pixel).
	int lineHeight = mFontTexture->GetHeight() / mLineCount;
	mGlyphHeight = lineHeight - 1; //TODO: Take baseline value into account?
	
	// We'll start processing glyphs at (1, 0).
	unsigned int currentX = 1;
	unsigned int currentY = 0;
	int currentLine = 1;
	
	// We'll now interate the font characters and determine the UV rects
	// within the font texture used to render each glyph.
	for(size_t i = 0; i < mFontCharacters.size(); i++)
	{
		Glyph glyph;
		glyph.character = mFontCharacters[i];
		
		// The width is at least 1.
		int width = 1;
		
		// The left X UV coord is just where we left off for the last glyph.
		// But right X UV requires us to find edge of next glyph (or end of texture).
		float leftUvX = (float)currentX / mFontTexture->GetWidth();
		for(currentX = currentX + 1; currentX < mFontTexture->GetWidth(); currentX++)
		{
			Color32 color = mFontTexture->GetPixelColor32(currentX, currentY);
			if(color == glyphStartColor) { break; }
			
			// This pixel is black, so it is part of the glyph's width.
			width++;
		}
		float rightUvX = (float)currentX / mFontTexture->GetWidth();
		
		// Save height/width.
		glyph.width = width;
		glyph.height = mGlyphHeight;
		
		// Calculate top/bottom y UV values.
		// TODO: Why is UV (0, 0) in top-left corner? Kind of unexpected...
		float botUvY = (float)(currentY + lineHeight) / mFontTexture->GetHeight();
		float topUvY = (float)(currentY + 1.0f) / mFontTexture->GetHeight();
		
		// Save those UV coords.
		glyph.bottomLeftUvCoord = Vector2(leftUvX, botUvY);
		glyph.topRightUvCoord = Vector2(rightUvX, topUvY);
		
		glyph.topLeftUvCoord = Vector2(leftUvX, topUvY);
		glyph.bottomRightUvCoord = Vector2(rightUvX, botUvY);
		
		// Save the glyph, mapped to its character.
		mFontGlyphs[glyph.character] = glyph;
		
		// If we reached the end of the font texture, go to the next line, if possible.
		if(currentX >= mFontTexture->GetWidth() - 1 && currentLine < mLineCount)
		{
			currentLine++;
			currentX = 1;
			currentY += lineHeight;
		}
	}
	
	// Update font texture with background color transparency.
	//mFontTexture->SetTransparentColor(backgroundColor);
	
	/*
	// Outputs glyphs for verification.
	std::cout << "Glyphs for font " << GetName() << std::endl;
	for(auto& glyph : mFontGlyphs)
	{
		std::cout << glyph.second.character << ": " << glyph.second.width <<
		", " << glyph.second.height << std::endl;
	}
	*/
}

Glyph& Font::GetGlyph(char character)
{
	auto it = mFontGlyphs.find(character);
	if(it != mFontGlyphs.end())
	{
		return it->second;
	}
	else
	{
		return mFontGlyphs[mDefaultChar];
	}
}

Shader* Font::GetShader() const
{
	if(mColorReplacement)
	{
		return Services::GetAssets()->LoadShader("3D-Diffuse-Tex", "UI-Text-ColorReplace");
	}
	return Material::sDefaultShader;
}

void Font::ParseFromData(char* data, int dataLength)
{
	// Font is in INI format, but only one key per line.
	IniParser parser(data, dataLength);
	parser.SetMultipleKeyValuePairsPerLine(false);
	
	// Read each line, and each key/pair, one at a time.
	while(parser.ReadLine())
	{
		while(parser.ReadKeyValuePair())
		{
			const IniKeyValue& keyValue = parser.GetKeyValue();
			if(StringUtil::EqualsIgnoreCase(keyValue.key, "font"))
			{
				//TODO: If font contains non-ASCII chars (pretty likely), this won't work for those chars.
				mFontCharacters = keyValue.value;
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "bitmap name"))
			{
				mFontTexture = Services::GetAssets()->LoadTexture(keyValue.value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "alpha channel"))
			{
				//TODO
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "line count"))
			{
				mLineCount = keyValue.GetValueAsInt();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "char extra"))
			{
				mExtraCharacterSpacing = keyValue.GetValueAsInt();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "line extra"))
			{
				mExtraLineSpacing = keyValue.GetValueAsInt();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "default char"))
			{
				mDefaultChar = keyValue.GetValueAsInt();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
			{
                // Default is "alpha blend," but it can still be specified explicitly.
                if(StringUtil::EqualsIgnoreCase(keyValue.value, "alpha blend"))
                {
                    mColorMode = ColorMode::AlphaBlend;
                }
				if(StringUtil::EqualsIgnoreCase(keyValue.value, "color replacement"))
				{
                    mColorMode = ColorMode::ColorReplace;
				}
                else
                {
                    std::cout << "Unknown font type: " << keyValue.value << std::endl;
                }
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "color"))
			{
				// Defined when type is "Color Replacement".
				mForegroundColor = keyValue.GetValueAsColor32();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "foreground color"))
			{
				// Defined when type is "Alpha Blend".
				mForegroundColor = keyValue.GetValueAsColor32();
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "background color"))
			{
				// Defined when type is "Alpha Blend".
				mBackgroundColor = keyValue.GetValueAsColor32();
			}
			else
			{
				std::cout << "Unknown font property: " << keyValue.key << std::endl;
			}
		}
	}
	
	// After parsing, if font texture is null, we'll fall back on using
	// a font texture with the same name as the font itself.
	if(mFontTexture == nullptr)
	{
		mFontTexture = Services::GetAssets()->LoadTexture(GetNameNoExtension());
	}
}
