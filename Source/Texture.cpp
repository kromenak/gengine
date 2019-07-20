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
#include "Math.h"

Texture* Texture::White = nullptr;
Texture* Texture::Black = nullptr;

void Texture::Init()
{
	White = new Texture(2, 2, Color32::White);
	Black = new Texture(2, 2, Color32::Black);
}

Texture::Texture(unsigned int width, unsigned int height, Color32 color) :
	Asset(""), mWidth(width), mHeight(height)
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
	
	GenerateOpenGlTexture();
}

Texture::Texture(std::string name, char* data, int dataLength) :
    Asset(name)
{
    ParseFromData(data, dataLength);
    GenerateOpenGlTexture();
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
	if(mPixels != nullptr)
	{
		delete[] mPixels;
		mPixels = nullptr;
	}
}

void Texture::Activate()
{
    glBindTexture(GL_TEXTURE_2D, mTextureId);
}

void Texture::Deactivate()
{
	// When we "deactivate" all textures, we'll just actually
	// activate our default "white" texture.
	if(White != nullptr)
	{
		White->Activate();
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
	}
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
	
	// Update texture data on GPU.
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
				 0, 0, mWidth, mHeight,
				 GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
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
	int index = (y * mWidth + x) * 4;
	
	// If index isn't valid...also return black.
	if(index < 0 || index >= (mWidth * mHeight * 4)) { return Color32::Black; }
	
	//float r = mPixels[index] / 255.0f;
	//float g = mPixels[index + 1] / 255.0f;
	//float b = mPixels[index + 2] / 255.0f;
	//float a = mPixels[index + 3] / 255.0f;
	//return Vector4(r, g, b, a);
	
	unsigned char r = mPixels[index];
	unsigned char g = mPixels[index + 1];
	unsigned char b = mPixels[index + 2];
	unsigned char a = mPixels[index + 3];
	return Color32(r, g, b, a);
}

void Texture::WriteToFile(std::string filePath)
{
    BinaryWriter writer(filePath.c_str());
    
    // BMP HEADER
    writer.WriteString("BM");
    writer.WriteUInt(0);    // Size of file in bytes. Optional to fill in.
    writer.WriteUShort(0);  // Reserved/empty
    writer.WriteUShort(0);  // Reserved/empty
    writer.WriteUInt(54);   // Offset to image data
    
    // DIB HEADER
    writer.WriteUInt(40); // Size of this header, always 40 bytes.
    writer.WriteInt(mWidth);
    writer.WriteInt(mHeight);
    writer.WriteUShort(1); // Number of color planes, always 1.
    writer.WriteUShort(32); // Number of bits-per-pixel. Assume 32 for now.
    writer.WriteUInt(0); // Compression method - assumed none (0).
    writer.WriteUInt(0); // Uncompressed size of image. Since we aren't compressing, can just use 0 as placeholder.
    writer.WriteInt(0); // Preferred width for printing, unused.
    writer.WriteInt(0); // Preferred height for printing, unused.
    writer.WriteUInt(0); // Number of palette colors, unused.
    writer.WriteUInt(0); // Number of important colors, unused.
    
    // COLOR TABLE - Only needed for 8BPP or less.
    
    // PIXELS
	// Write out one row at a time, bottom to top, left to right, per BMP format standard.
	int rowSize = CalculateBmpRowSize(32, mWidth);
    for(int y = mHeight - 1; y >= 0; y--)
    {
		int bytesWritten = 0;
        for(int x = 0; x < mWidth; x++)
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

void Texture::GenerateOpenGlTexture()
{
	// Generate and bind the texture object in OpenGL.
	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	
	// Load texture data into texture object.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				 mWidth, mHeight, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
	
	// When texturing, use the nearest pixel to pick the color to use.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void Texture::ParseFromData(char* data, int dataLength)
{
    BinaryReader reader(data, dataLength);
	
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
	// The compressed format has a second value here.
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
	for(int y = 0; y < mHeight; y++)
	{
		for(int x = 0; x < mWidth; x++)
		{
			int current = (y * mWidth + x) * 4;
			uint16_t pixel = reader.ReadUShort();
			
			float red = (pixel & 0xF800) >> 11;
			float green = (pixel & 0x07E0) >> 5;
			float blue = (pixel & 0x001F);
			
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
		// The order of the colors by default is blue, green, red, alpha.
		mPalette = new unsigned char[numColorsInColorPalette * 4];
		reader.Read(mPalette, numColorsInColorPalette * 4);
	}
	
	// PIXELS
	// Allocate pixels array.
	mPixels = new unsigned char[mWidth * mHeight * 4];
	
	// Read in pixel data.
	int rowSize = CalculateBmpRowSize(bitsPerPixel, mWidth);
	for(int y = mHeight - 1; y >= 0; y--)
	{
		int bytesRead = 0;
		for(int x = 0; x < mWidth; x++)
		{
			// Calculate index into pixels array.
			int index = (y * mWidth + x) * 4;
			
			// How we interpret pixel data will depend on the bpp.
			if(bitsPerPixel == 8)
			{
				// Read in the palette index. Multiply by 4 because each index has 4 bytes.
				// So 0 = 0, 1 = 4, 2 = 8, etc.
				int paletteIndex = reader.ReadUByte();
				paletteIndex *= 4;
				bytesRead++;
				
				if(mPalette != nullptr)
				{
					// Palette color order is BGRA. But our internal pixels are RGBA.
					mPixels[index] = mPalette[paletteIndex + 2];
					mPixels[index + 1] = mPalette[paletteIndex + 1];
					mPixels[index + 2] = mPalette[paletteIndex];
					
					// As long as the BMP format is BI_RGB, we can assume the image does not have any alpha data.
					// In these cases, the alpha value is usually zero.
					// But we actually want to interpret that as 255 (full alpha).
					mPixels[index + 3] = 255; //palette[paletteIndex + 3];
				}
			}
			else if(bitsPerPixel == 32)
			{
				// We're assuming pixel data order is RGBA.
				mPixels[index + 2] = reader.ReadUByte(); // Blue
				mPixels[index + 1] = reader.ReadUByte(); // Green
				mPixels[index] = reader.ReadUByte(); 	 // Red
				bytesRead += 3;
				
				// When BMP format is BI_RGB, there is no alpha.
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

/*static*/ int Texture::CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width)
{
	// Calculate number of bytes that should be present in each row.
	// Each row has 4-byte alignment, so this rounds us up to nearest 4 bytes.
	// For a 32-bit (4bpp) image...this doesn't really matter. But for other bpp sizes, it would.
	return Math::FloorToInt((bitsPerPixel * width + 31.0f) / 32.0f) * 4;
}
