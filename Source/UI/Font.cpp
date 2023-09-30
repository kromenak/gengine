#include "Font.h"

#include "AssetManager.h"
#include "Color32.h"
#include "IniParser.h"
#include "Material.h"
#include "StringUtil.h"
#include "Texture.h"

void Font::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);

    // After parsing, if we have no font texture, we can't do much more.
    if(mFontTexture == nullptr) { return; }

    // The pixel color at (1, 0) seems to always be glyph indicator color.
    Color32 glyphStartColor = mFontTexture->GetPixelColor32(1, 0);

    // The font texture may have the font glyphs in multiple vertical lines.
    // The height of each line can be calculated pretty easily.
    int lineHeight = mFontTexture->GetHeight() / mLineCount;

    // The glyph height is slightly smaller than the line height.
    // The top pixel of each line is a "glyph indicator", used to determine where each glyph starts (ignore it).
    mGlyphHeight = lineHeight - 1;

    // We'll start processing glyphs at (1, 0).
    unsigned int currentX = 1;
    unsigned int currentY = 0;
    int currentLine = 1;

    // For each char, determine UV rect within the font texture for rendering the glyph.
    for(size_t i = 0; i < mFontCharacters.size(); i++)
    {
        Glyph glyph;
        glyph.character = mFontCharacters[i];
        glyph.width = 1; // all glyphs are at least 1 pixel wide
        glyph.height = mGlyphHeight;

        // The left X UV coord is just where we left off from the last glyph.
        float leftUvX = (float)currentX / mFontTexture->GetWidth();

        // To find the right X UV coord, we need to find the edge of the next glyph (or end of the texture).
        bool foundGlyphStartColor = false;
        for(currentX = currentX + 1; currentX < mFontTexture->GetWidth(); ++currentX)
        {
            Color32 color = mFontTexture->GetPixelColor32(currentX, currentY);
            if(color == glyphStartColor)
            {
                foundGlyphStartColor = true;
                break;
            }

            // This pixel is black, so it is part of the glyph's width.
            ++glyph.width;
        }

        // There is a mild complication in GK3 fonts we need to account for.
        // If this is a SINGLE LINE font, and we get to end of texture WITHOUT finding "glyph start color", that's fine - use it.
        // But if this is a MULTI LINE font, we MUST find the "glyph start color". If not found, we go to next line.
        if(mLineCount > 1 && !foundGlyphStartColor)
        {
            // Go to next line.
            ++currentLine;
            currentX = 1;
            currentY += lineHeight;

            // If we get past the bottom of the font texture, break out of loop - quit trying to find glyphs.
            // Can happen due to data errors - ex: SID_PDN_12 font lists more chars in .FON file than there are glyphs in the BMP asset!
            if(currentY > mFontTexture->GetHeight())
            {
                break;
            }

            // We need to "redo" this glyph on the next line.
            --i;
            continue;
        }

        // We now know where the right UV coord is for this glyph.
        float rightUvX = (float)currentX / mFontTexture->GetWidth();

        // Calculate top/bottom UV values.
        // Top is +1 because the top pixel is discarded.
        float botUvY = (float)(currentY + lineHeight) / mFontTexture->GetHeight();
        float topUvY = (float)(currentY + 1.0f) / mFontTexture->GetHeight();

        // Save those UV coords.
        //TODO: Because this is a Rect, we're saving excess data here. Really only need 4 vals (min/max for x/y).
        glyph.bottomLeftUvCoord = Vector2(leftUvX, botUvY);
        glyph.topRightUvCoord = Vector2(rightUvX, topUvY);

        glyph.topLeftUvCoord = Vector2(leftUvX, topUvY);
        glyph.bottomRightUvCoord = Vector2(rightUvX, botUvY);

        // Save the glyph, mapped to its character.
        mFontGlyphs[glyph.character] = glyph;

        // If we reached the end of the font texture, go to the next line, if possible.
        if(currentX >= mFontTexture->GetWidth() - 1 && currentLine < mLineCount)
        {
            ++currentLine;
            currentX = 1;
            currentY += lineHeight;
        }
    }

    // If font uses color replace logic, the pixel at (0, 1)
    // indicates what color should be replaced with the foreground color.
    if(mColorMode == ColorMode::ColorReplace)
    {
        mReplaceColor = mFontTexture->GetPixelColor32(0, 1);
    }

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
	if(mColorMode == ColorMode::ColorReplace)
	{
		return gAssetManager.LoadShader("3D-Tex", "UI-Text-ColorReplace");
	}
	return Material::sDefaultShader;
}

void Font::ParseFromData(uint8_t* data, uint32_t dataLength)
{
	// Font is in INI format, but only one key per line.
	IniParser parser(data, dataLength);
	parser.SetMultipleKeyValuePairsPerLine(false);
	
	// Read each line, and each key/pair, one at a time.
    Texture* alphaTexture = nullptr;
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
				mFontTexture = gAssetManager.LoadTexture(keyValue.value, GetScope());
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "alpha channel"))
			{
                alphaTexture = gAssetManager.LoadTexture(keyValue.value, GetScope());
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
                mDefaultChar = static_cast<unsigned char>(keyValue.GetValueAsInt());
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "type"))
			{
                // Default is "alpha blend," but it can still be specified explicitly.
                if(StringUtil::EqualsIgnoreCase(keyValue.value, "alpha blend"))
                {
                    mColorMode = ColorMode::AlphaBlend;
                }
				else if(StringUtil::EqualsIgnoreCase(keyValue.value, "color replacement"))
				{
                    mColorMode = ColorMode::ColorReplace;
				}
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "color"))
			{
				// Defined when type is "Color Replacement".
                // The value is sometimes empty, which means we should just use the default.
                if(!keyValue.value.empty())
                {
                    mColor = keyValue.GetValueAsColor32();
                }
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "foreground color"))
			{
				// Defined when type is "Alpha Blend".
                // The value is sometimes empty, which means we should just use the default.
                if(!keyValue.value.empty())
                {
                    mColor = keyValue.GetValueAsColor32();
                }
			}
			else if(StringUtil::EqualsIgnoreCase(keyValue.key, "background color"))
			{
				// Defined when type is "Alpha Blend".
                // The value is sometimes empty, which means we should just use the default.
                if(!keyValue.value.empty())
                {
                    mBackgroundColor = keyValue.GetValueAsColor32();
                }
			}
            //TODO: function
            //TODO: destination opacity
            //TODO: source opacity
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
		mFontTexture = gAssetManager.LoadTexture(GetNameNoExtension(), GetScope());
	}

    // If we have an alpha channel apply it to the font texture.
    if(mFontTexture != nullptr && alphaTexture != nullptr)
    {
        mFontTexture->ApplyAlphaChannel(*alphaTexture);
    }
}
