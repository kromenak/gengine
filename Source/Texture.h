//
// Texture.h
//
// Clark Kromenaker
//
// 2D texture asset type. The in-memory
// representation of .BMP assets.
//
#pragma once
#include "Asset.h"

#include <GL/glew.h>
#include <string>

#include "Color32.h"

class BinaryReader;
class SDL_Surface;

class Texture : Asset
{
public:
	static Texture* White;
	static Texture* Black;
	
	static void Init();
	
	Texture(unsigned int width, unsigned int height, Color32 color);
    Texture(std::string name, char* data, int dataLength);
	~Texture();
	
	// Activates the texture in the graphics library.
    void Activate();
    static void Deactivate();
	
	void SetTransparentColor(Color32 color);
	
	// For SDL cursor stuff, convert texture to a surface.
	//TODO: Probably move this elsewhere?
    SDL_Surface* GetSurface();
    SDL_Surface* GetSurface(int x, int y, int width, int height);
    
    unsigned int GetWidth() { return mWidth; }
    unsigned int GetHeight() { return mHeight; }
    unsigned char* GetPixelData() const { return mPixels; }
	
	Color32 GetPixelColor32(int x, int y);
	
    void WriteToFile(std::string filePath);
    
private:
    // Texture width and height.
    unsigned int mWidth = 0;
    unsigned int mHeight = 0;
    
    // Pixel data - this is the meat of the texture!
    unsigned char* mPixels = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mTextureId = GL_NONE;
	
	void GenerateOpenGlTexture();
	
    void ParseFromData(char* data, int dataLength);
	void ParseFromCompressedFormat(BinaryReader& reader);
	void ParseFromBmpFormat(BinaryReader& reader);
};
