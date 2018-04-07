//
// Texture.h
//
// Clark Kromenaker
//
// 2D texture asset type. The in-memory
// representation of .BMP assets.
//
#pragma once
#include <string>
#include "Asset.h"
#include <GL/glew.h>

class Texture : Asset
{
public:
    Texture(std::string name, char* data, int dataLength);
    
    void Activate();
    static void Deactivate();
    
    unsigned int GetWidth() { return mWidth; }
    unsigned int GetHeight() { return mHeight; }
    unsigned char* GetPixelData() const { return mPixels; }
    
private:
    // Texture width and height.
    unsigned int mWidth = 0;
    unsigned int mHeight = 0;
    
    // Pixel data - this is the meat of the texture!
    unsigned char* mPixels = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mTextureId = GL_NONE;
    
    void ParseFromData(char* data, int dataLength);
};
