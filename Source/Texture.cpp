//
// Texture.cpp
//
// Clark Kromenaker
//
#include "Texture.h"
#include "BinaryReader.h"
#include <iostream>

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

void Texture::Activate()
{
    glBindTexture(GL_TEXTURE_2D, mTextureId);
}

void Texture::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier
    unsigned int fileIdentifier = reader.ReadUInt();
    if(fileIdentifier != 0x4D6E3136)
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
            mPixels[current + 3] = 255;
        }
        
        // Might need to skip some padding here.
        if((mWidth & 0x00000001) != 0)
        {
            reader.ReadUShort();
        }
    }
}
