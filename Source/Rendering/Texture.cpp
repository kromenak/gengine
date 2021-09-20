//
// Texture.cpp
//
// Clark Kromenaker
//
#include "Texture.h"

#include <iostream>

#include <SDL2/SDL.h>

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "GMath.h"

Texture Texture::White(2, 2, Color32::White);
Texture Texture::Black(2, 2, Color32::Black);

Texture::Texture(unsigned int width, unsigned int height) :
    Asset(""),
    mWidth(width),
    mHeight(height)
{
    // Create pixel array of desired size.
    int pixelsSize = mWidth * mHeight * 4;
    mPixels = new unsigned char[pixelsSize];
}

Texture::Texture(unsigned int width, unsigned int height, Color32 color) :
	Asset(""),
	mWidth(width),
	mHeight(height)
{
	// Create pixel array of desired size.
	int pixelsSize = mWidth * mHeight * 4;
	mPixels = new unsigned char[pixelsSize];
	
	// Flood-fill pixels with desired color.
	for(int i = 0; i < pixelsSize; i += 4)
	{
		mPixels[i] = color.GetR();
		mPixels[i + 1] = color.GetG();
		mPixels[i + 2] = color.GetB();
		mPixels[i + 3] = color.GetA();
	}
}

Texture::Texture(std::string name, char* data, int dataLength) :
    Asset(name)
{
	BinaryReader reader(data, dataLength);
    ParseFromData(reader);
}

Texture::Texture(BinaryReader& reader) :
    Asset("")
{
    ParseFromData(reader);
}

Texture::~Texture()
{
	if(mTextureId != GL_NONE)
	{
		glDeleteTextures(1, &mTextureId);
	}
	if(mPalette != nullptr)
	{
		delete[] mPalette;
		mPalette = nullptr;
	}
	if(mPaletteIndexes != nullptr)
	{
		delete[] mPaletteIndexes;
		mPaletteIndexes = nullptr;
	}
	if(mPixels != nullptr)
	{
		delete[] mPixels;
		mPixels = nullptr;
	}
}

void Texture::Activate(int textureUnit)
{
    // Activate desired texture unit.
    // Only do this if desired unit is not already active! Small performance gain.
    static int activeTextureUnit = -1;
    if(activeTextureUnit != textureUnit)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        activeTextureUnit = textureUnit;
    }

    // Upload to GPU if dirty.
    if(mDirty)
    {
        UploadToGPU();
        mDirty = false;
    }

    // Bind texture (again, if not already bound - small performance gain).
    static GLuint activeTextureId[2];
    if(activeTextureId[textureUnit] != mTextureId)
    {
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        activeTextureId[textureUnit] = mTextureId;
    }
}

/*static*/ void Texture::Deactivate()
{
    White.Activate(0);
}

SDL_Surface* Texture::GetSurface()
{
    return GetSurface(0, 0, mWidth, mHeight);
}

SDL_Surface* Texture::GetSurface(int x, int y, int width, int height)
{
    unsigned int rmask, gmask, bmask, amask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
    #else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    #endif
    
    int depth = 32;
    int pitch = 4 * width;
    
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)mPixels, width, height, depth, pitch,
                                                    rmask, gmask, bmask, amask);
    if(surface == nullptr)
    {
        SDL_Log("Creating surface failed: %s", SDL_GetError());
    }
    return surface;
}

Color32 Texture::GetPixelColor32(int x, int y)
{
	// No pixels means...just return black.
	if(mPixels == nullptr) { return Color32::Black; }
	
	// Calculate index into pixels array.
	unsigned int index = static_cast<unsigned int>((y * mWidth + x) * 4);
	
	// If index isn't valid...also return black.
	if(index < 0 || index >= (mWidth * mHeight * 4)) { return Color32::Black; }
	
	unsigned char r = mPixels[index];
	unsigned char g = mPixels[index + 1];
	unsigned char b = mPixels[index + 2];
	unsigned char a = mPixels[index + 3];
	return Color32(r, g, b, a);
}

unsigned char Texture::GetPaletteIndex(int x, int y)
{
	// No palette indexes means we can't get a value!
	if(mPaletteIndexes == nullptr) { return 0; }
	
	// Calculate index into pixels array.
	unsigned int index = static_cast<unsigned int>(y * mWidth + x);
	
	// If index isn't valid...also return zero.
	if(index < 0 || index >= (mWidth * mHeight)) { return 0; }
	
	// Got it!
	return mPaletteIndexes[index];
}

/*
//CK: Tried this for face texture mixing. This works, BUT it doesn't properly blend alpha. But maybe useful for something, some day?
void Texture::Blit(Texture* source, int destX, int destY)
{
	GLuint fboId = GL_NONE;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source->mTextureId, 0);
	
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, destX, destY, 0, 0, source->GetWidth(), source->GetHeight());
	
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glDeleteFramebuffers(1, &fboId);
}
*/
 
void Texture::BlendPixels(const Texture& source, Texture& dest, int destX, int destY)
{
	BlendPixels(source, 0, 0, source.mWidth, source.mHeight, dest, destX, destY);
}

void Texture::BlendPixels(const Texture& source, int sourceX, int sourceY, int sourceWidth, int sourceHeight,
					     Texture& dest, int destX, int destY)
{
	// We can't copy out-of-bounds pixels from the source.
	if(sourceX < 0 || sourceX >= static_cast<int>(source.mWidth)) { return; }
	if(sourceY < 0 || sourceY >= static_cast<int>(source.mHeight)) { return; }
	
	// We can't copy to out-of-bounds pixels in the destination.
	if(destX < 0 || destX >= static_cast<int>(dest.mWidth)) { return; }
	if(destY < 0 || destY >= static_cast<int>(dest.mHeight)) { return; }
	
	// Brute force copy, pixel by pixel!
	for(int y = sourceY; y < sourceY + sourceHeight && y < static_cast<int>(source.mHeight); ++y)
	{
		for(int x = sourceX; x < sourceX + sourceWidth && x < static_cast<int>(source.mWidth); ++x)
		{
			// Calculate source pixel index.
			int sourcePixelIndex = (y * source.mWidth + x) * 4;
			
			// Calculate dest x/y for this pixel.
			int copyToY = destY + (y - sourceY);
			int copyToX = destX + (x - sourceX);
			if(copyToY < 0 || copyToY >= dest.mHeight) { continue; }
			if(copyToX < 0 || copyToY >= dest.mWidth) { continue; }
			
			// Calculate dest pixel index based on x/y.
			int destPixelIndex = (copyToY * dest.mWidth + copyToX) * 4;
			
			// Interpolate between source/dest pixel colors based on source alpha value.
			// If source alpha is zero, use 100% dest color - this value will be 0.
			// If source alpha is 255, use 100% source color - this value will be 1.
			// If source alpha is between, use X% dest/source color - this value is 0-1.
			float alphaPercent = (float)source.mPixels[sourcePixelIndex + 3] / 255.0f;
			
			// Copy!
			dest.mPixels[destPixelIndex] = Math::Lerp(dest.mPixels[destPixelIndex], source.mPixels[sourcePixelIndex], alphaPercent);
			dest.mPixels[destPixelIndex + 1] = Math::Lerp(dest.mPixels[destPixelIndex + 1], source.mPixels[sourcePixelIndex + 1], alphaPercent);
			dest.mPixels[destPixelIndex + 2] = Math::Lerp(dest.mPixels[destPixelIndex + 2], source.mPixels[sourcePixelIndex + 2], alphaPercent);
			// Don't make any changes to dest's alpha channel.
		}
	}
	
	// Don't upload dest to GPU here, since we might be doing a bunch of copy operations in a row.
	// We'll leave it up to the caller to do that manually (for now).
}

void Texture::SetTransparentColor(Color32 color)
{
	if(mPixels == nullptr) { return; }
	
	// Find instances of the desired transparent color and
	// make sure the alpha value is zero.
	int pixelByteCount = mWidth * mHeight * 4;
	for(int i = 0; i < pixelByteCount; i += 4)
	{
		if(mPixels[i] == color.GetR() &&
		   mPixels[i + 1] == color.GetG() &&
		   mPixels[i + 2] == color.GetB())
		{
			mPixels[i + 3] = 0;
		}
		else
		{
			mPixels[i + 3] = 255;
		}
	}
	
    // Mark dirty so it uploads to GPU on next use.
    mDirty = true;
}

void Texture::ApplyAlphaChannel(const Texture& alphaTexture)
{
	// For now, let's assume alpha texture has same width/height as target texture.
	if(alphaTexture.mWidth != mWidth || alphaTexture.mHeight != mHeight)
	{
		std::cout << "Can't apply alpha texture! Width and height do not match." << std::endl;
		return;
	}
	
	// If the alpha texture has a palette, we want to treat the R/G/B values as the alpha value.
	// Palettized textures as alpha channels usually have palette colors like (255, 255, 255, 0) or (128, 128, 128, 0).
	// At least, that's the case in GK3!
	bool useRgbForAlpha = alphaTexture.mPalette != nullptr;
	
	// For each pixel, copy over the alpha value.
	int pixelCount = mWidth * mHeight;
	for(int i = 0; i < pixelCount; ++i)
	{
		// If RGB is alpha value, just grab R val. Otherwise, +3 to get A val.
		unsigned char alpha = useRgbForAlpha ? alphaTexture.mPixels[(i * 4)] : alphaTexture.mPixels[(i * 4) + 3];
		mPixels[(i * 4) + 3] = alpha;
	}
}

void Texture::UploadToGPU()
{
	if(mTextureId == GL_NONE)
	{
		// Generate and bind the texture object in OpenGL.
		glGenTextures(1, &mTextureId);
		glBindTexture(GL_TEXTURE_2D, mTextureId);
		
		// Load texture data into texture object.
        // OpenGL assumes that pixel data is from bottom-left, BUT our pixels array is from top-left!
        // You'd think this would lead to upside-down textures in-game...BUT GK3 uses DirectX style UVs (from top-left).
        // So, this "double inversion" actually leads to textures displaying correctly in OpenGL.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
					 mWidth, mHeight, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
		
		// Set filter mode for the texture.
        GLfloat filterParam = mFilterMode == FilterMode::Point ? GL_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterParam);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterParam);
        
        // Set wrap mode for the texture.
        GLfloat wrapParam = mWrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
	}
	else
	{
		// Update texture data on GPU.
		glBindTexture(GL_TEXTURE_2D, mTextureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0,
						0, 0, mWidth, mHeight,
						GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
	}
}

void Texture::WriteToFile(std::string filePath)
{
    BinaryWriter writer(filePath.c_str());
    
    // BMP HEADER
    writer.WriteString("BM");
    writer.WriteUInt(0);    // Size of file in bytes. Optional to fill in.
    writer.WriteUShort(0);  // Reserved/empty
    writer.WriteUShort(0);  // Reserved/empty
    writer.WriteUInt(54);   // Offset to image data (from beginning of this file)
    
    // DIB HEADER
    writer.WriteUInt(40);       // Size of this header, always 40 bytes.
    writer.WriteInt(mWidth);    // Width of image; signed for some reason.
    writer.WriteInt(mHeight);   // Height of image; signed for some reason.
    writer.WriteUShort(1);      // Number of color planes, always 1.
    writer.WriteUShort(32);     // Number of bits-per-pixel. Assume 32 for now.
    writer.WriteUInt(0);        // Compression method - assumed none (0).
    writer.WriteUInt(0);        // Uncompressed size of image. Since we aren't compressing, can just use 0 as placeholder.
    writer.WriteInt(0);         // Preferred width for printing, unused.
    writer.WriteInt(0);         // Preferred height for printing, unused.
    writer.WriteUInt(0);        // Number of palette colors, unused.
    writer.WriteUInt(0);        // Number of important colors, unused.
    
    // COLOR TABLE - Only needed for 8BPP or less.
    
    // PIXELS
	// Write out one row at a time, bottom to top, left to right, per BMP format standard.
	int rowSize = CalculateBmpRowSize(32, mWidth);
    for(int y = mHeight - 1; y >= 0; --y)
    {
		int bytesWritten = 0;
        for(int x = 0; x < mWidth; ++x)
        {
            int index = (y * mWidth + x) * 4;
			writer.WriteUByte(mPixels[index + 2]); // Blue
            writer.WriteUByte(mPixels[index + 1]); // Green
			writer.WriteUByte(mPixels[index]); 	   // Red
            writer.WriteUByte(0); // Alpha is ignored
			bytesWritten += 4;
        }
		
		// Add padding to write out total desired row size (padded to 4 bytes).
		while(bytesWritten < rowSize)
		{
			writer.WriteUByte(0);
			bytesWritten++;
		}
    }
}

/*static*/ int Texture::CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width)
{
	// Calculate number of bytes that should be present in each row.
	// Each row has 4-byte alignment, so this rounds us up to nearest 4 bytes.
	// For a 32-bit (4bpp) image...this doesn't really matter. But for other bpp sizes, it would.
	return Math::FloorToInt((bitsPerPixel * width + 31.0f) / 32.0f) * 4;
}

void Texture::ParseFromData(BinaryReader &reader)
{
    // Texture can be in one of two formats:
    // 1) A custom/compressed format.
    // 2) A normal BMP format.
    // The first 2 byte value can tell us.
    unsigned short fileIdentifier = reader.ReadUShort();
    if(fileIdentifier == 0x3136) // 16
    {
        ParseFromCompressedFormat(reader);
    }
    else if(fileIdentifier == 0x4D42) // BM
    {
        ParseFromBmpFormat(reader);
    }
}

void Texture::ParseFromCompressedFormat(BinaryReader& reader)
{
    // 2 bytes: compressed file identifier (assumed this has already been read in from constructor).
	// 2 bytes: The compressed format has a second value here.
	unsigned short fileIdentifier2 = reader.ReadUShort();
	if(fileIdentifier2 != 0x4D6E) // Mn
	{
		std::cout << "BMP file does not have correct identifier!" << std::endl;
		return;
	}
	
	// Read width and height values.
	mHeight = reader.ReadUShort();
	mWidth = reader.ReadUShort();
	
	// Allocate pixels array.
	mPixels = new unsigned char[mWidth * mHeight * 4];
    
    // Read in pixel data.
    // This pixel data is stored top-left to bottom-right, so we don't flip (our pixel array starts at top-left corner).
	for(int y = 0; y < mHeight; ++y)
	{
		for(int x = 0; x < mWidth; ++x)
		{
			int current = (y  * mWidth + x) * 4;
			uint16_t pixel = reader.ReadUShort();
			
			float red = static_cast<float>((pixel & 0xF800) >> 11);
			float green = static_cast<float>((pixel & 0x07E0) >> 5);
			float blue = static_cast<float>((pixel & 0x001F));
            
			mPixels[current] = (unsigned char)(red * 255 / 31);
			mPixels[current + 1] = (unsigned char)(green * 255 / 63);
			mPixels[current + 2] = (unsigned char)(blue * 255 / 31);
			
			// Causes all instances of magenta (R = 255, B = 255) to appear transparent.
			if(mPixels[current] > 200 && mPixels[current + 1] < 100 && mPixels[current + 2] > 200)
			{
				mPixels[current + 3] = 0;
			}
			else
			{
				mPixels[current + 3] = 255;
			}
		}
		
		// Might need to skip some padding here.
		if((mWidth & 0x00000001) != 0)
		{
			reader.ReadUShort();
		}
	}
	
	// This seeeeems to work consistently - if the top-left pixel has no alpha, flag as alpha test.
	if(mHeight > 0 && mWidth > 0 && mPixels[3] == 0)
	{
		mRenderType = RenderType::AlphaTest;
	}
}

void Texture::ParseFromBmpFormat(BinaryReader& reader)
{
	// BMP HEADER
    // 2 bytes: BMP file identifier (assumed this has already been read in from constructor).
	// 4 bytes: size of file in bytes
	// 4 bytes: 2 shorts that are reserved/unused
	// 4 bytes: offset to image data
	reader.Skip(12);
	
	// DIB HEADER
	// 4 bytes: size of DIB header (always 40)
	unsigned int dibHeaderSize = reader.ReadUInt();
	if(dibHeaderSize != 40)
	{
		std::cout << "Texture: unsupported dib header size of " << dibHeaderSize << std::endl;
		return;
	}
	
	// 8 bytes: width and height
	mWidth = reader.ReadUInt();
	mHeight = reader.ReadUInt();
	
	// 2 bytes: number of color planes
	unsigned short colorPlaneCount = reader.ReadUShort();
	if(colorPlaneCount != 1)
	{
		std::cout << "Texture: unsupported color plane count of " << colorPlaneCount << std::endl;
		return;
	}
	
	// 2 bytes: number of bits per pixel
	unsigned short bitsPerPixel = reader.ReadUShort();
	
	// 4 bytes: compression method
	// 0 = BI_RGB (not compressed)
	// 1 = BI_RLE8 (RLE 8-bit/pixel)
	// 2 = BI_RLE4 (RLE 4-bit/pixel)
	// 3 = BI_BITFIELDS (???)
	// 4 = BI_JPEG (a JPEG image)
	// 5 = BI_PNG (a PNG image)
	// 6-13 = BI_ALPHABITFIELDS, BI_CMYK, BI_CMYKRLE8, BI_CMYKRLE4
	unsigned int compressionMethod = reader.ReadUInt();
	if(compressionMethod != 0)
	{
		std::cout << "Texture: unsupported compression method " << compressionMethod << std::endl;
		return;
	}
	
	// 4 bytes: uncompressed size; but if compression method is zero, this is usually also zero (unset).
	// 8 bytes: horizontal/vertical resolution (pixels per meter) - unused.
	reader.Skip(12);
	
	// 4 bytes: num colors in palette. If zero, default to 2^(bpp)
	unsigned int numColorsInColorPalette = reader.ReadUInt();
	if(numColorsInColorPalette == 0)
	{
		numColorsInColorPalette = Math::PowBase2(bitsPerPixel);
	}
	
	// 4 bytes: num important colors - unused.
	reader.Skip(4);
	
	// COLOR TABLE - only present for 8-bpp or lower images
	if(bitsPerPixel <= 8)
	{
		// The number of bytes is numColors in palette, time 4 bytes each.
		// The order of the colors is blue, green, red, alpha.
		mPalette = new unsigned char[numColorsInColorPalette * 4];
		reader.Read(mPalette, numColorsInColorPalette * 4);
		
		/*
		std::cout << GetName() << std::endl;
		for(int i = 0; i < numColorsInColorPalette; i++)
		{
			Color32 color(mPalette[i * 4 + 2], mPalette[i * 4 + 1], mPalette[i * 4], mPalette[i * 4 + 3]);
			std::cout << i << ", " << color << std::endl;
		}
		*/
	}
	
	// PIXELS
	// Allocate pixels array.
	mPixels = new unsigned char[mWidth * mHeight * 4];
	
	// For 8-bpp or lower images with a palette, allocate palette indexes.
	if(bitsPerPixel <= 8)
	{
		mPaletteIndexes = new unsigned char[mWidth * mHeight];
	}
	
	// Read in pixel data.
    // BMP pixel data is stored bottom-left to top-right, so we do flip (our pixel array starts at top-left corner).
	int rowSize = CalculateBmpRowSize(bitsPerPixel, mWidth);
	for(int y = mHeight - 1; y >= 0; --y)
	{
		int bytesRead = 0;
		for(unsigned int x = 0; x < mWidth; ++x)
		{
			// Calculate index into pixels array.
            int index = (y * mWidth + x) * 4;
			
			// How we interpret pixel data will depend on the bpp.
			if(bitsPerPixel == 8)
			{
				// Read in the palette index and save it.
				int paletteIndex = reader.ReadUByte();
				mPaletteIndexes[(y * mWidth + x)] = paletteIndex;
				bytesRead++;
				
				//TODO: For palettized textures, should we hold off on creating pixels array until someone requests it?
				if(mPalette != nullptr)
				{
					// Since each palette color has 4 bytes, multiply by 4 to get byte offset.
					int paletteByteIndex = paletteIndex * 4;
					
					// Palette color order is BGRA. But our internal pixels are RGBA.
					mPixels[index] = mPalette[paletteByteIndex + 2];
					mPixels[index + 1] = mPalette[paletteByteIndex + 1];
					mPixels[index + 2] = mPalette[paletteByteIndex];
					
					// As long as the BMP format is BI_RGB, we can assume the image does not have any alpha data.
					// In these cases, the alpha value is usually zero.
					// But we actually want to interpret that as 255 (full alpha).
					mPixels[index + 3] = 255; //palette[paletteByteIndex + 3];
				}
			}
			else if(bitsPerPixel == 24 || bitsPerPixel == 32)
			{
                // Assuming BI_RGB format, alpha is not stored.
                // So regardless of bits per pixel of 24 or 32, the data layout and size is the same.
                
				// Pixel data in the BMP file is BGR.
                // Internal pixel data is RGBA, so reorganize on read in.
				mPixels[index + 2] = reader.ReadUByte(); // Blue
				mPixels[index + 1] = reader.ReadUByte(); // Green
				mPixels[index] = reader.ReadUByte(); 	 // Red
				bytesRead += 3;
				
				// BI_RGB format doesn't save any alpha, even if 32 bits per pixel.
				// We'll use a placeholder of 255 (full alpha).
				mPixels[index + 3] = 255; // Alpha
			}
			else
			{
				std::cout << "Texture: Unaccounted for BPP of " << bitsPerPixel << std::endl;
			}
		}
		
		// Skip padding that may be present, to ensure 4-byte alignment.
		if(bytesRead < rowSize)
		{
			reader.Skip(rowSize - bytesRead);
		}
	}
}
