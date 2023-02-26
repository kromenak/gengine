//
// Clark Kromenaker
//
// 2D texture asset type.
// The in-memory representation of .BMP assets.
//
#pragma once
#include "Asset.h"

#include <string>

#include <GL/glew.h>

#include "Atomics.h"
#include "Color32.h"
#include "EnumClassFlags.h"

class BinaryReader;

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
        Bilinear,   // Average nearby pixels when up close or far away. Blurs the texture a bit.
        Trilinear   // Averages between mipmap levels. If mipmaps aren't enabled, falls back on bilinear filtering.
    };
    
    // Dictates how the texture acts when U/V outside of normal 0-1 bounds are accessed.
    enum class WrapMode
    {
        Repeat,     // Repeat the texture.
        Clamp       // Clamp to 0-1 range.
    };

    // Keep track of what data needs to be uploaded to the GPU.
    enum class DirtyFlags : int
    {
        None = 0,       // Nothing is dirty.
        Pixels = 1,     // Pixel data needs to be re-uploaded to the GPU.
        Mipmaps = 2,    // Mipmaps have been enabled or disabled.
        Properties = 4, // Other properties (filter mode, wrap mode, etc) have changed.
    };
	
	static Texture White;
	static Texture Black;
	
    Texture(uint32 width, uint32 height);
	Texture(uint32 width, uint32 height, Color32 color);
    Texture(const std::string& name, char* data, uint32 dataLength);
    Texture(BinaryReader& reader);
	~Texture();
	
	// Activates the texture in the graphics library.
    void Activate(int textureUnit);
    static void Deactivate();
    
    uint32 GetWidth() const { return mWidth; }
    uint32 GetHeight() const { return mHeight; }
    uint8* GetPixelData() const { return mPixels; }
	
	RenderType GetRenderType() const { return mRenderType; }
	
    void SetFilterMode(FilterMode filterMode);
    FilterMode GetFilterMode() const { return mFilterMode; }
    
    void SetWrapMode(WrapMode wrapMode);
    WrapMode GetWrapMode() const { return mWrapMode; }

    void SetMipmaps(bool useMipmaps);
    
    // Coordinates are from top-left corner of texture.
    void SetPixelColor32(int x, int y, const Color32& color);
	Color32 GetPixelColor32(int x, int y) const;

    void SetPaletteIndex(int x, int y, uint8 val);
	uint8 GetPaletteIndex(int x, int y) const;
	
	//void Blit(Texture* source, int destX, int destY);
	
	// Blend's source pixels into dest based on source's alpha channel.
	static void BlendPixels(const Texture& source, Texture& dest, int destX, int destY);
    static void BlendPixels(const Texture& source, int sourceX, int sourceY, int sourceWidth, int sourceHeight,
                            Texture& dest, int destX, int destY);
	
	// Alpha and transparency
	void SetTransparentColor(const Color32& color);
    void ClearTransparentColor();
	void ApplyAlphaChannel(const Texture& alphaTexture);

    // GPU upload
    void AddDirtyFlags(DirtyFlags flags);
	void UploadToGPU();

    // Export/save
	void WriteToFile(const std::string& filePath);
	
private:
	friend class RenderTexture; // To access OpenGL stuff.
	
    // Texture width and height.
    uint32 mWidth = 0;
    uint32 mHeight = 0;
	
	// Some textures have palettes.
	uint8* mPalette = nullptr;
    uint32 mPaletteSize = 0;
	
	// If a texture has a palette, the indexes into the palette are stored here.
    uint8* mPaletteIndexes = nullptr;
	
    // Pixel data, from the top-left corner of the image.
    // SDL and DirectX (I think) expect pixel data from top-left corner.
    // OpenGL expects from bottom-left, but we compensate for that by using flipped UVs!
    uint8* mPixels = nullptr;
    
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

    // If true, this texture generates and uses mipmaps when rendering.
    bool mMipmaps = false;

    // Flags indicating that texture data in RAM is dirty, so we need to upload to GPU.
    // A newly created texture will automatically have its "dirty pixels" flag set, since we must upload pixel data before use.
    DirtyFlags mDirtyFlags = DirtyFlags::Pixels;
	
	static int CalculateBmpRowSize(unsigned short bitsPerPixel, unsigned int width);
	
    void ParseFromData(BinaryReader& reader);
	void ParseFromCompressedFormat(BinaryReader& reader);
	void ParseFromBmpFormat(BinaryReader& reader);
};

ENUM_CLASS_FLAGS(Texture::DirtyFlags);
