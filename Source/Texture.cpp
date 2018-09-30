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

Texture::Texture(std::string name, char* data, int dataLength) :
    Asset(name)
{
    // Retrieve the data.
    ParseFromData(data, dataLength);
    
    // Generate and bind the texture object in OpenGL.
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    
    // Load texture data into texture object.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 mWidth, mHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
    
    //TODO: Should put this in initialization or rendering or...?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Texture::~Texture()
{
	if(mTextureId != GL_NONE)
	{
		glDeleteTextures(1, &mTextureId);
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
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
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

void Texture::WriteToFile(std::string filePath)
{
    BinaryWriter writer(filePath.c_str());
    
    // BMP HEADER
    writer.WriteString("BM");
    writer.WriteUInt(0);    // Size of file in bytes
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
    
    // COLOR TABLE - Don't need it?
    
    // PIXELS
    //int rowSize = Math::FloorToInt((32.0f * mWidth + 31.0f) / 32.0f) * 4;
    for(int y = mHeight - 1; y >= 0; y--)
    {
        for(int x = 0; x < mWidth; x++)
        {
            int index = (y * mWidth + x) * 4;
            writer.WriteUByte(mPixels[index]);
            writer.WriteUByte(mPixels[index + 1]);
            writer.WriteUByte(mPixels[index + 2]);
            writer.WriteUByte(0); // Alpha is ignored
        }
        
        // Add padding if we need it.
        if((mWidth & 0x00000001) != 0)
        {
            writer.WriteUShort(0);
        }
    }
}

void Texture::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
	
	// Texture can be in one of two formats:
	// 1) A custom/compressed format.
	// 2) A normal BMP format.
	// The first 2 bytes can tell us.
	unsigned short fileIdentifier = reader.ReadUShort();
	if(fileIdentifier == 0x3136) // 16
	{
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
	}
	else if(fileIdentifier == 0x4D42) // BM
	{
		// BMP HEADER
		// 4 bytes: size of file in bytes
		// 4 bytes: 2 shorts that are reserved/unused
		reader.Skip(8);
		
		// 4 bytes: offset to image data
		reader.Skip(4);
		
		// DIB HEADER
		// 4 bytes: size of DIB header (always 40)
		reader.Skip(4);
		
		// 8 bytes: width and height (12 x 37)
		mWidth = reader.ReadUInt();
		mHeight = reader.ReadUInt();
		
		// 2 bytes: number of color planes
		unsigned short colorPlaneCount = reader.ReadUShort();
		
		// 2 bytes: number of bits per pixel
		unsigned short bitsPerPixel = reader.ReadUShort();
		
		// 8 bytes: compression method (0 if not compressed).
		// If not compressed, uncompressed size is usually zero.
		unsigned int compressionMethod = reader.ReadUInt();
		unsigned int uncompressedSize = reader.ReadUInt();
		
		// 16 bytes: 4 unused reserved ints
		reader.Skip(16);
		
		// COLOR TABLE - Don't need it?
		
		// PIXELS
		// Allocate pixels array.
		mPixels = new unsigned char[mWidth * mHeight * 4];
		
		for(int y = mHeight - 1; y >= 0; y--)
		{
			for(int x = 0; x < mWidth; x++)
			{
				int index = (y * mWidth + x) * 4;
				writer.WriteUByte(mPixels[index]);
				writer.WriteUByte(mPixels[index + 1]);
				writer.WriteUByte(mPixels[index + 2]);
				writer.WriteUByte(0); // Alpha is ignored
			}
			
			// Add padding if we need it.
			if((mWidth & 0x00000001) != 0)
			{
				writer.WriteUShort(0);
			}
		}
	}
}
