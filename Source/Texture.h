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
    
    // Dictates how the texture acts when it is magnified or minified.
    enum class FilterMode
    {
        Point,      // Use nearest neighbor when up close or far away. Can result in pixelated textures.
        Bilinear    // Average nearby pixels when up close or far away. Blurs the texture a bit.
    };
    
    // Dictates how the texture acts when U/V outside of normal 0-1 bounds are accessed.
    enum class WrapMode
    {
        Repeat,     // Repeat the texture.
        Clamp       // Clamp to 0-1 range.
    };
	
	static Texture White;
	static Texture Black;
	
	Texture(unsigned int width, unsigned int height, Color32 color);
    Texture(std::string name, char* data, int dataLength);
    Texture(BinaryReader& reader);
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
	
    void SetFilterMode(FilterMode filterMode) { mFilterMode = filterMode; }
    FilterMode GetFilterMode() const { return mFilterMode; }
    
    void SetWrapMode(WrapMode wrapMode) { mWrapMode = wrapMode; }
    WrapMode GetWrapMode() const { return mWrapMode; }
    
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
	
    // Pixel data, from the top-left corner of the image.
    // SDL and DirectX (I think) expect pixel data from top-left corner.
    // OpenGL expects from bottom-left, but we compensate for that by using flipped UVs!
    unsigned char* mPixels = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mTextureId = GL_NONE;
	
	// If there's no alpha, it is an opaque texture.
	// If it has alpha, but only 255 or 0 (on or off), it's an alpha test texture.
	// If it has semi-alpha pixels, it is a translucent texture.
	RenderType mRenderType = RenderType::Opaque;
    
    // Texture's filter mode.
    FilterMode mFilterMode = FilterMode::Point;

    // Texture's wrap mode.
    WrapMode mWrapMode = WrapMode::Repeat;
    
    // If true, texture data in RAM is dirty, so we need to upload to GPU.
    bool mDirty = true;
	
	static int CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width);
	
    void ParseFromData(BinaryReader& reader);
	void ParseFromCompressedFormat(BinaryReader& reader);
	void ParseFromBmpFormat(BinaryReader& reader);
};
