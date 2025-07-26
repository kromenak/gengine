//
// Clark Kromenaker
//
// 2D texture asset type.
// The in-memory representation of .BMP assets.
//
#pragma once
#include "Asset.h"

#include <cstdint>
#include <string>

#include "Color32.h"
#include "EnumClassFlags.h"

class BinaryReader;

class Texture : public Asset
{
    TYPEINFO_SUB(Texture, Asset);
public:
    enum class RenderType
    {
        Opaque,     // Texture is fully opaque
        AlphaTest,  // Texture has only opaque and transparent pixels
        Translucent // Texture has pixels that are partially transparent
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

    enum class Format : uint8_t
    {
        // 24 bpp
        BGR,
        RGB,

        // 32 bpp
        BGRA,
        RGBA,
    };

    static Texture White;
    static Texture Black;

    Texture(uint32_t width, uint32_t height, Format format = Format::RGBA);
    Texture(uint32_t width, uint32_t height, Color32 color, Format format = Format::RGBA);
    Texture(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    Texture(BinaryReader& reader);
    ~Texture();

    void Load(uint8_t* data, uint32_t dataLength);

    // Activates the texture in the graphics library.
    void Activate(uint8_t textureUnit);
    static void Deactivate(uint8_t textureUnit);

    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }

    uint8_t GetBytesPerPixel() const { return mBytesPerPixel; }
    Format GetFormat() const { return mFormat; }

    uint8_t* GetPixelData() const { return mPixels; }

    // Sampling/Rendering Properties
    //TODO: It may make sense to move this stuff up to the material layer?
    RenderType GetRenderType() const { return mRenderType; }

    void SetFilterMode(FilterMode filterMode);
    FilterMode GetFilterMode() const { return mFilterMode; }

    void SetWrapMode(WrapMode wrapMode);
    WrapMode GetWrapMode() const { return mWrapMode; }

    void SetMipmaps(bool useMipmaps);

    // Set or Get Pixels
    // Coordinates are from top-left corner of texture.
    void SetAllPixelsColor(Color32 color);
    void SetPixelColor(uint32_t x, uint32_t y, Color32 color);
    void SetPixelColor(uint32_t pixelIndex, Color32 color);
    Color32 GetPixelColor(uint32_t x, uint32_t y) const;
    Color32 GetPixelColor(uint32_t pixelIndex) const;

    // Set or Get Palette Indexes
    void SetPixelPaletteIndex(uint32_t x, uint32_t y, uint8_t val);
    uint8_t GetPixelPaletteIndex(uint32_t x, uint32_t y) const;

    // Blend's source pixels into dest based on source's alpha channel.
    static void BlendPixels(const Texture& source, Texture& dest, uint32_t destX, uint32_t destY);
    static void BlendPixels(const Texture& source, uint32_t sourceX, uint32_t sourceY, uint32_t sourceWidth, uint32_t sourceHeight,
                            Texture& dest, uint32_t destX, uint32_t destY);

    // Alpha and Transparency
    //TODO: The idea of a "transparent color" may make more sense at the material layer? See gDiscardColor in our shaders.
    void SetTransparentColor(const Color32& color);
    void ClearTransparentColor();
    void ApplyAlphaChannel(const Texture& alphaTexture);

    // Image Modifications
    void FlipVertically();
    void FlipHorizontally();
    void RotateClockwise();
    void RotateCounterclockwise();
    void Resize(uint32_t width, uint32_t height);
    void Crop(uint32_t width, uint32_t height, bool centered = false);
    void Crop(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    // GPU upload
    void AddDirtyFlags(DirtyFlags flags);
    void UploadToGPU();

    // Export/save
    void WriteToFile(const std::string& filePath);

private:
    // Texture width and height.
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    // The format of the pixel data. This matches exactly the data stored in the pixels array.
    Format mFormat = Format::RGBA;

    // The number of bytes per pixel for this image.
    // 4 => 32-bit RGBA image, 3 => 24-bit RGB image, 1 => 8-bit palettized image.
    uint8_t mBytesPerPixel = 4;

    // Pixel data, from the TOP-LEFT CORNER of the image.
    uint8_t* mPixels = nullptr;

    // A palette storing RGBA colors.
    // Palettized images don't store pixel data directly. Instead, each pixel specifies an index into this palette.
    uint8_t* mPalette = nullptr;
    uint32_t mPaletteSize = 0;

    // If a texture has a palette, the indexes into the palette are stored here.
    // By iterating this and populating the associated pixels array with the color from the palette, you can convert a palettized image to an RGBA one.
    uint8_t* mPaletteIndexes = nullptr;

    // Handle to texture in underlying graphics API.
    void* mTextureHandle = nullptr;

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

    void LoadInternal(BinaryReader& reader);
    void LoadCompressedFormat(BinaryReader& reader);
    void LoadBmpFormat(BinaryReader& reader);
    void LoadPngFormat(BinaryReader& reader);

    void CreatePixelsFromPaletteData();
};

ENUM_CLASS_FLAGS(Texture::DirtyFlags);