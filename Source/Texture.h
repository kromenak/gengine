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
	/*
	enum class Format
	{
		RGB,
		RGBA
	};
	*/
	
	enum class RenderType
	{
		Opaque,
		AlphaTest,
		Translucent
	};
	
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
    
    unsigned int GetWidth() const { return mWidth; }
    unsigned int GetHeight() const { return mHeight; }
	
    unsigned char* GetPixelData() const { return mPixels; }
	
	bool HasAlpha() { return mHasAlpha; }
	RenderType GetRenderType() const { return mRenderType; }
	
	Color32 GetPixelColor32(int x, int y);
	
    void WriteToFile(std::string filePath);
	
	void Blit(Texture* source, int destX, int destY);
	
	// Blend's source pixels into dest based on source's alpha channel.
	static void BlendPixels(const Texture& source, Texture& dest, int destX, int destY);
	static void BlendPixels(const Texture& source, int sourceX, int sourceY, int sourceWidth, int sourceHeight,
						   Texture& dest, int destX, int destY);
	
	void UploadToGPU();
	
	void ApplyAlphaChannel(const Texture& alphaTexture);
	
private:
	friend class RenderTexture; // To access OpenGL stuff.
	
    // Texture width and height.
    unsigned int mWidth = 0;
    unsigned int mHeight = 0;
	
	// Some textures have palettes.
	unsigned char* mPalette = nullptr;
	
    // Pixel data - this is the meat of the texture!
    unsigned char* mPixels = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mTextureId = GL_NONE;
	
	// Textures tend to have a "render type". If there's no alpha, it is an opaque texture.
	// If it has alpha, but only 255 or 0, it's an alpha test texture.
	// If it has semi-alpha pixels, it is a translucent texture.
	RenderType mRenderType = RenderType::Opaque;
	
	// If true, the texture has alpha, so it may need to be rendered
	bool mHasAlpha = false;
	bool mIsTranslucent = false;
	
    void ParseFromData(char* data, int dataLength);
	void ParseFromCompressedFormat(BinaryReader& reader);
	void ParseFromBmpFormat(BinaryReader& reader);
	
	static int CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width);
};
