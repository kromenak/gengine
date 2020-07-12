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
//#include <OpenGL/gl.h>
#include <string>

#include "Color32.h"

class BinaryReader;
struct SDL_Surface;

class Texture : public Asset
{
public:
	enum class RenderType
	{
		Opaque,		// Texture is fully opaque
		AlphaTest,	// Texture has only opaque and transparent pixels
		Translucent	// Texture has pixels that are partially transparent
	};
	
	static Texture White;
	static Texture Black;
	
	static void Init();
	
	Texture(unsigned int width, unsigned int height, Color32 color);
    Texture(std::string name, char* data, int dataLength);
	~Texture();
	
	// Activates the texture in the graphics library.
    void Activate(int textureUnit);
    static void Deactivate();
	
	// For SDL cursor stuff, convert texture to a surface.
	//TODO: Probably move this elsewhere?
    SDL_Surface* GetSurface();
    SDL_Surface* GetSurface(int x, int y, int width, int height);
    
    unsigned int GetWidth() const { return mWidth; }
    unsigned int GetHeight() const { return mHeight; }
    unsigned char* GetPixelData() const { return mPixels; }
	
	RenderType GetRenderType() const { return mRenderType; }
	
    // Coordinates are from top-left corner of texture.
	Color32 GetPixelColor32(int x, int y);
	unsigned char GetPaletteIndex(int x, int y);
	
	//void Blit(Texture* source, int destX, int destY);
	
	// Blend's source pixels into dest based on source's alpha channel.
	static void BlendPixels(const Texture& source, Texture& dest, int destX, int destY);
	static void BlendPixels(const Texture& source, int sourceX, int sourceY, int sourceWidth, int sourceHeight,
						   Texture& dest, int destX, int destY);
	
	// Alpha and transparency
	void SetTransparentColor(Color32 color);
	void ApplyAlphaChannel(const Texture& alphaTexture);
	
	void UploadToGPU();
	
	void WriteToFile(std::string filePath);
	
private:
	friend class RenderTexture; // To access OpenGL stuff.
	
    // Texture width and height.
    unsigned int mWidth = 0;
    unsigned int mHeight = 0;
	
	// Some textures have palettes.
	unsigned char* mPalette = nullptr;
	
	// If a texture has a palette, the indexes into the palette are stored here.
	unsigned char* mPaletteIndexes = nullptr;
	
    // Pixel data - this is the meat of the texture!
    unsigned char* mPixels = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mTextureId = GL_NONE;
	
	// If there's no alpha, it is an opaque texture.
	// If it has alpha, but only 255 or 0 (on or off), it's an alpha test texture.
	// If it has semi-alpha pixels, it is a translucent texture.
	RenderType mRenderType = RenderType::Opaque;
    
    // If true, texture data in RAM is dirty, so we need to upload to GPU.
    bool mDirty = true;
	
	static int CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width);
	
	void ParseFromCompressedFormat(BinaryReader& reader);
	void ParseFromBmpFormat(BinaryReader& reader);
};
