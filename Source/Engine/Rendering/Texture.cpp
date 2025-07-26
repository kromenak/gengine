#include "Texture.h"

#include <cstring>

#include <stb_image_resize.h>

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "PNGCodec.h"
#include "ThreadUtil.h"

namespace
{
    int CalculateBmpRowSize(uint16_t bitsPerPixel, uint32_t width)
    {
        // Calculate number of bytes that should be present in each row.
        // Each row has 4-byte alignment, so this rounds us up to nearest 4 bytes.
        // For a 32-bit (4bpp) image...this doesn't really matter. But for other bpp sizes, it would.
        return Math::FloorToInt((bitsPerPixel * width + 31.0f) / 32.0f) * 4;
    }

    uint8_t GetFormatBytesPerPixel(Texture::Format format)
    {
        switch(format)
        {
            case Texture::Format::BGR:
            case Texture::Format::RGB:
                return 3;

            case Texture::Format::BGRA:
            case Texture::Format::RGBA:
            default:
                return 4;
        }
    }

}

TYPEINFO_INIT(Texture, Asset, GENERATE_TYPE_ID)
{
    TYPEINFO_VAR(Texture, VariableType::Int, mWidth);
    TYPEINFO_VAR(Texture, VariableType::Int, mHeight);
    TYPEINFO_VAR(Texture, VariableType::Bool, mMipmaps);
}

Texture Texture::White(1, 1, Color32::White);
Texture Texture::Black(1, 1, Color32::Black);

Texture::Texture(uint32_t width, uint32_t height, Format format) : Asset("", AssetScope::Manual),
    mWidth(width),
    mHeight(height),
    mFormat(format),
    mBytesPerPixel(GetFormatBytesPerPixel(mFormat))
{
    // Create pixel array of desired size.
    size_t pixelsSize = mWidth * mHeight * mBytesPerPixel;
    mPixels = new uint8_t[pixelsSize];
}

Texture::Texture(uint32_t width, uint32_t height, Color32 color, Format format) : Asset("", AssetScope::Manual),
    mWidth(width),
    mHeight(height),
    mFormat(format),
    mBytesPerPixel(GetFormatBytesPerPixel(mFormat))
{
    // Create pixel array of desired size.
    int pixelsSize = mWidth * mHeight * mBytesPerPixel;
    mPixels = new uint8_t[pixelsSize];

    // Flood-fill pixels with desired color.
    SetAllPixelsColor(color);
}

Texture::Texture(BinaryReader& reader) : Asset("", AssetScope::Manual)
{
    LoadInternal(reader);
}

Texture::~Texture()
{
    // We can't 100% guarantee all textures are destroyed on main thread, and some graphics APIs are sensitive to that.
    // So, queue destruction to always happen on the main thread.
    if(mTextureHandle != nullptr)
    {
        void* texHandle = mTextureHandle;
        ThreadUtil::RunOnMainThread([texHandle]() {
            GAPI::Get()->DestroyTexture(texHandle);
        });
    }

    delete[] mPixels;
    mPixels = nullptr;

    delete[] mPalette;
    mPalette = nullptr;

    delete[] mPaletteIndexes;
    mPaletteIndexes = nullptr;
}

void Texture::Load(uint8_t* data, uint32_t dataLength)
{
    BinaryReader reader(data, dataLength);
    LoadInternal(reader);
}

void Texture::Activate(uint8_t textureUnit)
{
    // Make sure we're operating on the correct texture unit, first of all.
    // If this isn't set first, operations in UploadToGPU *may* errantly unbind an existing texture on the current texture unit.
    // This can result in weird render states (e.g. using newly uploaded lightmaps as color textures unintentionally).
    GAPI::Get()->SetTextureUnit(textureUnit);

    // Upload to GPU if dirty.
    UploadToGPU();

    // If UploadToGPU had work to do, it probably already did this.
    // But to be 100% sure, make sure the texture is active/bound to the texture unit!
    if(mTextureHandle != nullptr)
    {
        GAPI::Get()->ActivateTexture(mTextureHandle);
    }
}

/*static*/ void Texture::Deactivate(uint8_t textureUnit)
{
    // We always want to have a valid texture active, to avoid garbage rendering if we accidentally render without an active texture.
    // So always fall back on the white texture, worst case.
    White.Activate(textureUnit);
}

void Texture::SetFilterMode(FilterMode filterMode)
{
    mFilterMode = filterMode;
    mDirtyFlags |= DirtyFlags::Properties;
}

void Texture::SetWrapMode(WrapMode wrapMode)
{
    mWrapMode = wrapMode;
    mDirtyFlags |= DirtyFlags::Properties;
}

void Texture::SetMipmaps(bool useMipmaps)
{
    mMipmaps = useMipmaps;
    mDirtyFlags |= DirtyFlags::Mipmaps;
}

void Texture::SetAllPixelsColor(Color32 color)
{
    // If this is a palettized image, attempting to change a pixel color triggers conversion to pixel data.
    CreatePixelsFromPaletteData();

    // Must have pixels to continue.
    if(mPixels != nullptr)
    {
        // Iterate all pixels, setting the color for each.
        for(uint32_t pixelIndex = 0; pixelIndex < mWidth * mHeight; ++pixelIndex)
        {
            uint32_t pixelByteIndex = pixelIndex * mBytesPerPixel;
            if(mFormat == Format::BGR || mFormat == Format::BGRA)
            {
                mPixels[pixelByteIndex] = color.b;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.r;
            }
            else
            {
                mPixels[pixelByteIndex] = color.r;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.b;
            }

            if(mBytesPerPixel >= 4)
            {
                mPixels[pixelByteIndex + 3] = color.a;
            }
        }

        // This dirties the pixel.
        mDirtyFlags |= DirtyFlags::Pixels;
    }
}

void Texture::SetPixelColor(uint32_t x, uint32_t y, Color32 color)
{
    // Make sure the x/y passed in are valid.
    uint32_t pixelIndex = y * mWidth + x;
    if(pixelIndex >= mWidth * mHeight) { return; }

    // Pass off to index version.
    SetPixelColor(pixelIndex, color);
}

void Texture::SetPixelColor(uint32_t pixelIndex, Color32 color)
{
    // If this is a palettized image, attempting to change a pixel color triggers conversion to pixel data.
    CreatePixelsFromPaletteData();

    // Need pixels to do this.
    if(mPixels != nullptr)
    {
        // Update pixel based on format.
        uint32_t pixelByteIndex = pixelIndex * mBytesPerPixel;
        switch(mFormat)
        {
            case Format::BGR:
                mPixels[pixelByteIndex] = color.b;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.r;
                break;

            case Format::RGB:
                mPixels[pixelByteIndex] = color.r;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.b;
                break;

            case Format::BGRA:
                mPixels[pixelByteIndex] = color.b;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.r;
                mPixels[pixelByteIndex + 3] = color.a;
                break;

            case Format::RGBA:
                mPixels[pixelByteIndex] = color.r;
                mPixels[pixelByteIndex + 1] = color.g;
                mPixels[pixelByteIndex + 2] = color.b;
                mPixels[pixelByteIndex + 3] = color.a;
                break;
        }

        // This dirties the pixel.
        mDirtyFlags |= DirtyFlags::Pixels;
    }
}

Color32 Texture::GetPixelColor(uint32_t x, uint32_t y) const
{
    // Get pixel index; if not valid, we have to return something...how about black???
    uint32_t pixelIndex = y * mWidth + x;
    if(pixelIndex >= (mWidth * mHeight)) { return Color32::Black; }

    // Pass off to index version.
    return GetPixelColor(pixelIndex);
}

Color32 Texture::GetPixelColor(uint32_t pixelIndex) const
{
    // Whether this is a pixel-based texture or a palette-based texture, we can retrieve a pixel's color.
    // But each requires a slightly different approach.
    if(mPixels != nullptr)
    {
        // Get offset into pixel array of desired color.
        uint32_t pixelByteIndex = pixelIndex * mBytesPerPixel;

        // Return color correctly depending on pixel data format.
        switch(mFormat)
        {
            case Format::BGR:
                return Color32(mPixels[pixelByteIndex + 2],
                               mPixels[pixelByteIndex + 1],
                               mPixels[pixelByteIndex]);

            case Format::RGB:
                return Color32(mPixels[pixelByteIndex],
                               mPixels[pixelByteIndex + 1],
                               mPixels[pixelByteIndex + 2]);

            case Format::BGRA:
                return Color32(mPixels[pixelByteIndex + 2],
                               mPixels[pixelByteIndex + 1],
                               mPixels[pixelByteIndex],
                               mPixels[pixelByteIndex + 3]);

            case Format::RGBA:
                return Color32(mPixels[pixelByteIndex],
                               mPixels[pixelByteIndex + 1],
                               mPixels[pixelByteIndex + 2],
                               mPixels[pixelByteIndex + 3]);
        }
    }
    else if(mPalette != nullptr && mPaletteIndexes != nullptr)
    {
        // Palette data is always in BGRA format, but A component is unused.
        int paletteByteIndex = mPaletteIndexes[pixelIndex] * 4;
        return Color32(mPalette[paletteByteIndex + 2],
                       mPalette[paletteByteIndex + 1],
                       mPalette[paletteByteIndex]);
    }

    // This shouldn't happen, but return black worst case???
    return Color32::Black;
}

void Texture::SetPixelPaletteIndex(uint32_t x, uint32_t y, uint8_t val)
{
    // No palette indexes means we can't get a value!
    if(mPaletteIndexes == nullptr) { return; }

    // If index isn't valid, we also can't get a value.
    uint32_t index = y * mWidth + x;
    if(index >= mWidth * mHeight) { return; }

    // Set it!
    mPaletteIndexes[index] = val;
}

uint8_t Texture::GetPixelPaletteIndex(uint32_t x, uint32_t y) const
{
    // No palette indexes means we can't get a value!
    if(mPaletteIndexes == nullptr) { return 0; }

    // If index isn't valid...also return zero.
    uint32_t index = y * mWidth + x;
    if(index >= mWidth * mHeight) { return 0; }

    // Got it!
    return mPaletteIndexes[index];
}

/*static*/ void Texture::BlendPixels(const Texture& source, Texture& dest, uint32_t destX, uint32_t destY)
{
    BlendPixels(source, 0, 0, source.mWidth, source.mHeight, dest, destX, destY);
}

/*static*/ void Texture::BlendPixels(const Texture& source, uint32_t sourceX, uint32_t sourceY, uint32_t sourceWidth, uint32_t sourceHeight,
                                     Texture& dest, uint32_t destX, uint32_t destY)
{
    //TODO: If pixel formats aren't aligned (e.g. RGB source and BGR dest or vice-versa), this code will not give correct results.

    // We can't copy out-of-bounds pixels from the source.
    if(sourceX >= source.mWidth || sourceY >= source.mHeight) { return; }

    // We can't copy to out-of-bounds pixels in the destination.
    if(destX >= dest.mWidth || destY >= dest.mHeight) { return; }

    // Brute force copy, pixel by pixel!
    for(uint32_t y = sourceY; y < sourceY + sourceHeight && y < source.mHeight; ++y)
    {
        for(uint32_t x = sourceX; x < sourceX + sourceWidth && x < source.mWidth; ++x)
        {
            // Calculate source pixel byte index.
            uint32_t sourcePixelByteIndex = (y * source.mWidth + x) * source.mBytesPerPixel;

            // Calculate dest x/y for this pixel.
            uint32_t copyToX = destX + (x - sourceX);
            uint32_t copyToY = destY + (y - sourceY);
            if(copyToX >= dest.mWidth || copyToY >= dest.mHeight) { continue; }

            // Calculate dest pixel byte index.
            uint32_t destPixelByteIndex = (copyToY * dest.mWidth + copyToX) * dest.mBytesPerPixel;

            // Interpolate between source/dest pixel colors based on source alpha value.
            // If source alpha is zero, use 100% dest color - this value will be 0.
            // If source alpha is 255, use 100% source color - this value will be 1.
            // If source alpha is between, use X% dest/source color - this value is 0-1.
            float alphaPercent = 1.0f;
            if(source.mBytesPerPixel >= 4)
            {
                alphaPercent = static_cast<float>(source.mPixels[sourcePixelByteIndex + 3]) / 255.0f;
            }
            else if(source.mPixels[sourcePixelByteIndex] == 255 && source.mPixels[sourcePixelByteIndex + 2] == 255)
            {
                alphaPercent = 0.0f;
            }

            // Copy!
            dest.mPixels[destPixelByteIndex] = Math::Lerp(dest.mPixels[destPixelByteIndex], source.mPixels[sourcePixelByteIndex], alphaPercent);
            dest.mPixels[destPixelByteIndex + 1] = Math::Lerp(dest.mPixels[destPixelByteIndex + 1], source.mPixels[sourcePixelByteIndex + 1], alphaPercent);
            dest.mPixels[destPixelByteIndex + 2] = Math::Lerp(dest.mPixels[destPixelByteIndex + 2], source.mPixels[sourcePixelByteIndex + 2], alphaPercent);
        }
    }

    // Don't upload dest to GPU here, since we might be doing a bunch of copy operations in a row.
    // We'll leave it up to the caller to do that manually (for now).
    dest.mDirtyFlags |= DirtyFlags::Pixels;
}

void Texture::SetTransparentColor(const Color32& color)
{
    if(mPixels == nullptr || mBytesPerPixel < 4)
    {
        printf("ERROR: transparent color can only be set on 32-bit textures.\n");
        return;
    }

    // Find instances of the desired transparent color and
    // make sure the alpha value is zero.
    int pixelByteCount = mWidth * mHeight * mBytesPerPixel;
    for(int i = 0; i < pixelByteCount; i += mBytesPerPixel)
    {
        if(mFormat == Format::RGBA && mPixels[i] == color.r && mPixels[i + 1] == color.g && mPixels[i + 2] == color.b)
        {
            mPixels[i + 3] = 0;
        }
        else if(mFormat == Format::BGRA && mPixels[i] == color.b && mPixels[i + 1] == color.g && mPixels[i + 2] == color.r)
        {
            mPixels[i + 3] = 0;
        }
        else
        {
            mPixels[i + 3] = 255;
        }
    }

    // Mark dirty so it uploads to GPU on next use.
    mDirtyFlags |= DirtyFlags::Pixels;
}

void Texture::ClearTransparentColor()
{
    if(mPixels == nullptr || mBytesPerPixel < 4)
    {
        printf("ERROR: transparent color can only be cleared on 32-bit textures.\n");
        return;
    }

    // Make sure all pixels are opaque.
    int pixelByteCount = mWidth * mHeight * mBytesPerPixel;
    for(int i = 0; i < pixelByteCount; i += mBytesPerPixel)
    {
        mPixels[i + 3] = 255;
    }

    // Mark dirty so it uploads to GPU on next use.
    mDirtyFlags |= DirtyFlags::Pixels;
}

void Texture::ApplyAlphaChannel(const Texture& alphaTexture)
{
    // For now, let's assume alpha texture has same width/height as target texture.
    if(alphaTexture.mWidth != mWidth || alphaTexture.mHeight != mHeight)
    {
        printf("Can't apply alpha texture! Width and height do not match.\n");
        return;
    }

    // If no pixel data exists, this may be a palettized image - this is an operation that would trigger a conversion to pixel data.
    if(mPixels == nullptr)
    {
        CreatePixelsFromPaletteData();
        if(mPixels == nullptr)
        {
            printf("Can't apply alpha texture! No pixel data exists.\n");
            return;
        }
    }

    // Applying an alpha channel to this image implies it should have an alpha channel!
    // Convert if needed.
    if(mBytesPerPixel < 4)
    {
        uint8_t* newPixels = new uint8_t[mWidth * mHeight * 4];
        for(uint32_t pixelIndex = 0; pixelIndex < mWidth * mHeight; ++pixelIndex)
        {
            uint32_t srcByteOffset = pixelIndex * mBytesPerPixel;
            uint32_t dstByteOffset = pixelIndex * 4;

            newPixels[dstByteOffset] = mPixels[srcByteOffset];
            newPixels[dstByteOffset + 1] = mPixels[srcByteOffset + 1];
            newPixels[dstByteOffset + 2] = mPixels[srcByteOffset + 2];
            newPixels[dstByteOffset + 3] = 255;
        }

        delete[] mPixels;
        mPixels = newPixels;

        mBytesPerPixel = 4;
        switch(mFormat)
        {
            case Format::BGR:
                mFormat = Format::BGRA;
                break;
            case Format::RGB:
                mFormat = Format::RGBA;
                break;
            default:
                printf("Unexpected pixel format in ApplyAlphaChannel!\n");
                break;
        }

        mDirtyFlags |= DirtyFlags::Pixels;
    }

    // If the alpha texture has a palette, we want to treat the R/G/B values as the alpha value.
    // Palettized textures as alpha channels usually have palette colors like (255, 255, 255, 0) or (128, 128, 128, 0).
    // At least, that's the case in GK3!
    bool useRgbForAlpha = alphaTexture.mPalette != nullptr;

    // For each pixel, copy over the alpha value.
    uint32_t pixelCount = mWidth * mHeight;
    for(uint32_t i = 0; i < pixelCount; ++i)
    {
        // If RGB is alpha value, just grab R val. Otherwise, +3 to get A val.
        Color32 color = alphaTexture.GetPixelColor(i);
        mPixels[(i * mBytesPerPixel) + 3] = useRgbForAlpha ? color.r : color.a;
    }

    // Pixels are dirty.
    mDirtyFlags |= DirtyFlags::Pixels;

    // If an alpha channel is applied, we'll assume this texture is now translucent.
    mRenderType = RenderType::Translucent;
}

void Texture::FlipVertically()
{
    if(mPixels != nullptr)
    {
        // Iterate the top half of the image, swapping each row with its counterpart at the bottom of the image.
        uint32_t bytesPerRow = mWidth * mBytesPerPixel;
        uint8_t* temp = new uint8_t[bytesPerRow];
        for(uint32_t y = 0; y < mHeight / 2; ++y)
        {
            uint32_t otherY = mHeight - y - 1;

            uint8_t* rowAData = mPixels + (y * bytesPerRow);
            uint8_t* rowBData = mPixels + (otherY * bytesPerRow);

            // Let's try copying the entire line for better performance...
            // The trade-off is needing to allocate a temporary buffer for the swap.
            memcpy(temp, rowAData, bytesPerRow);
            memcpy(rowAData, rowBData, bytesPerRow);
            memcpy(rowBData, temp, bytesPerRow);
        }
        delete[] temp;

        // The pixels are dirty.
        mDirtyFlags |= DirtyFlags::Pixels;
    }
}

void Texture::FlipHorizontally()
{
    if(mPixels != nullptr)
    {
        // Go row by row and swap pixels across the center of each line.
        for(uint32_t y = 0; y < mHeight; ++y)
        {
            uint8_t* rowPixels = mPixels + (y * mWidth * mBytesPerPixel);
            for(uint32_t x = 0; x < mWidth / 2; ++x)
            {
                // Pixel data is either 3BPP or 4BPP. For 4BPP, we can interpret as a uint32 to do a single swap.
                // For 3BPP, there's no built-in 3-byte type. We *could* use a 3-byte struct, but just doing 3 swaps for now.
                if(mBytesPerPixel == 4)
                {
                    std::swap(reinterpret_cast<uint32_t*>(rowPixels)[x],
                              reinterpret_cast<uint32_t*>(rowPixels)[mWidth - x - 1]);
                }
                else
                {
                    uint32_t xByteOffset = x * mBytesPerPixel;
                    uint32_t otherByteOffset = (mWidth - 1 - x) * mBytesPerPixel;

                    std::swap(rowPixels[xByteOffset], rowPixels[otherByteOffset]);
                    std::swap(rowPixels[xByteOffset + 1], rowPixels[otherByteOffset + 1]);
                    std::swap(rowPixels[xByteOffset + 2], rowPixels[otherByteOffset + 2]);
                }
            }
        }

        // This dirties the pixels.
        mDirtyFlags |= DirtyFlags::Pixels;
    }
}

void Texture::RotateClockwise()
{
    if(mPixels != nullptr)
    {
        // We transpose the pixels "matrix," only processing the upper triangular portion.
        // This does rotate the image, BUT it leaves it mirrored as well...
        for(int y = 0; y < mHeight; ++y)
        {
            for(int x = y + 1; x < mWidth; ++x)
            {
                int offset1 = (y * mWidth + x);
                int offset2 = (x * mWidth + y);
                if(mBytesPerPixel == 4)
                {
                    std::swap(reinterpret_cast<uint32_t*>(mPixels)[offset1],
                              reinterpret_cast<uint32_t*>(mPixels)[offset2]);
                }
                else
                {
                    offset1 *= mBytesPerPixel;
                    offset2 *= mBytesPerPixel;

                    std::swap(mPixels[offset1], mPixels[offset2]);
                    std::swap(mPixels[offset1 + 1], mPixels[offset2 + 1]);
                    std::swap(mPixels[offset1 + 2], mPixels[offset2 + 2]);
                }
            }
        }

        // We can resolve the mirrored-ness by flipping (this also dirties the pixels for us).
        FlipHorizontally();
    }
}

void Texture::RotateCounterclockwise()
{
    if(mPixels != nullptr)
    {
        // Similar to above, transpose the upper triangular of the pixel matrix.
        // This is mirrored in the same way as rotating clockwise...
        for(int y = 0; y < mHeight; ++y)
        {
            for(int x = y + 1; x < mWidth; ++x)
            {
                int offset1 = (y * mWidth + x);
                int offset2 = (x * mWidth + y);
                if(mBytesPerPixel == 4)
                {
                    std::swap(reinterpret_cast<uint32_t*>(mPixels)[offset1],
                              reinterpret_cast<uint32_t*>(mPixels)[offset2]);
                }
                else
                {
                    offset1 *= mBytesPerPixel;
                    offset2 *= mBytesPerPixel;

                    std::swap(mPixels[offset1], mPixels[offset2]);
                    std::swap(mPixels[offset1 + 1], mPixels[offset2 + 1]);
                    std::swap(mPixels[offset1 + 2], mPixels[offset2 + 2]);
                }
            }
        }

        // But flipping vertically gives us a correct counter-clockwise rotation (and also dirties the pixels).
        FlipVertically();
    }
}

void Texture::Resize(uint32_t width, uint32_t height)
{
    if(mPixels != nullptr)
    {
        // Create a new pixel array at the desired size.
        uint8_t* newPixels = new uint8_t[width * height * mBytesPerPixel];

        // stb image library to the rescue...
        stbir_resize_uint8(mPixels, mWidth, mHeight, 0,
                           newPixels, width, height, 0, mBytesPerPixel);

        // Update width and height.
        mWidth = width;
        mHeight = height;

        // Replace pixels array.
        delete[] mPixels;
        mPixels = newPixels;

        // Just about everything is dirty.
        //TODO: Since the size changed, maybe we need to create a whole new texture here?
        AddDirtyFlags(DirtyFlags::Pixels | DirtyFlags::Properties | DirtyFlags::Mipmaps);
    }
}

void Texture::Crop(uint32_t width, uint32_t height, bool centered)
{
    if(centered)
    {
        Crop((mWidth / 2) - (width / 2), (mHeight / 2) - (height / 2), width, height);
    }
    else
    {
        Crop(0, 0, width, height);
    }
}

void Texture::Crop(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(mPixels != nullptr)
    {
        // You must crop within the current dimensions of the texture.
        assert(x < mWidth && y < mHeight && x + width <= mWidth && y + height <= mHeight);

        // Allocate new pixels for the updated size.
        uint8_t* newPixels = new uint8_t[width * height * mBytesPerPixel];

        // Copy pixels from old to new set of pixels.
        for(uint32_t dstY = 0; dstY < height; ++dstY)
        {
            for(uint32_t dstX = 0; dstX < width; ++dstX)
            {
                uint32_t srcX = x + dstX;
                uint32_t srcY = y + dstY;

                size_t srcOffset = ((srcY * mWidth) + srcX) * mBytesPerPixel;
                size_t dstOffset = ((dstY * width) + dstX) * mBytesPerPixel;

                newPixels[dstOffset] = mPixels[srcOffset];
                newPixels[dstOffset + 1] = mPixels[srcOffset + 1];
                newPixels[dstOffset + 2] = mPixels[srcOffset + 2];
                if(mBytesPerPixel >= 4)
                {
                    newPixels[dstOffset + 3] = mPixels[srcOffset + 3];
                }
            }
        }

        // Save updated width/height.
        mWidth = width;
        mHeight = height;

        // Replace pixels.
        delete[] mPixels;
        mPixels = newPixels;

        // Mark everything as dirty.
        //TODO: Since the size changed, maybe we need to create a whole new texture here?
        AddDirtyFlags(DirtyFlags::Pixels | DirtyFlags::Properties | DirtyFlags::Mipmaps);
    }
}

void Texture::AddDirtyFlags(DirtyFlags flags)
{
    mDirtyFlags |= flags;
}

void Texture::UploadToGPU()
{
    // Nothing to do.
    if(mDirtyFlags == DirtyFlags::None) { return; }

    // If attempting to upload a palettized texture to the GPU for the first time,
    // We need to convert the palette data to pixel data first.
    if(mPixels == nullptr && mPaletteIndexes != nullptr)
    {
        CreatePixelsFromPaletteData();
    }

    //TODO: To perform upload operations, this function must change the bound texture on the current texture unit.
    //TODO: As a result, it may be a good idea to pass in which texture unit to use?
    //TODO: Alternatively, this function could change the bound texture, but then change it back once it's done uploading...

    // If no texture handle yet, we must create a new texture in the underlying graphics API.
    if(mTextureHandle == nullptr)
    {
        // Create a texture and set pixels.
        mTextureHandle = GAPI::Get()->CreateTexture(mWidth, mHeight, mFormat, mPixels);

        // We must upload properties when texture is first generated too.
        mDirtyFlags |= DirtyFlags::Properties;
    }
    else
    {
        // If pixel data is dirty, upload new pixel data.
        if((mDirtyFlags & DirtyFlags::Pixels) != DirtyFlags::None)
        {
            GAPI::Get()->SetTexturePixels(mTextureHandle, mWidth, mHeight, mFormat, mPixels);

            // If using mipmaps, we must regenerate mipmaps for this texture after changing its pixels.
            if(mMipmaps)
            {
                GAPI::Get()->GenerateMipmaps(mTextureHandle);
            }
        }
    }

    // Deal with changes to mipmaps enabled/disabled.
    if((mDirtyFlags & DirtyFlags::Mipmaps) != DirtyFlags::None)
    {
        // If mipmaps have been enabled, generate mipmaps!
        // Note that if mipmaps have been disabled, we don't bother destroying the mipmaps - we just won't use them (see below).
        if(mMipmaps)
        {
            GAPI::Get()->GenerateMipmaps(mTextureHandle);
        }

        // Since trilinear filtering depends on mipmaps, we need to modify texture properties if trilinear filtering is enabled.
        if(mFilterMode == FilterMode::Trilinear)
        {
            mDirtyFlags |= DirtyFlags::Properties;
        }
    }

    // Deal with property changes.
    if((mDirtyFlags & DirtyFlags::Properties) != DirtyFlags::None)
    {
        // Set wrap mode for the texture.
        GAPI::Get()->SetTextureWrapMode(mTextureHandle, mWrapMode);

        // Set filter mode for texture.
        GAPI::Get()->SetTextureFilterMode(mTextureHandle, mFilterMode, mMipmaps);
    }

    // We did it all - clear the dirty flags.
    mDirtyFlags = DirtyFlags::None;
}

void Texture::WriteToFile(const std::string& filePath)
{
    if(Path::HasExtension(filePath, "png"))
    {
        PNG::ImageData imageData;
        imageData.width = mWidth;
        imageData.height = mHeight;
        imageData.bytesPerPixel = mBytesPerPixel;
        imageData.pixelData = mPixels;
        PNG::Encode(imageData, filePath.c_str());
    }
    else
    {
        BinaryWriter writer(filePath.c_str());

        // Determine the DIB header size. This depends on whether we need alpha support or not.
        // The most basic DIB header format (BITMAPINFOHEADER) is 40 bytes, but does not support an alpha channel.
        // To get alpha channel, BITMAPV4HEADER format is used, which is 108 bytes.
        int dibHeaderSize = mBytesPerPixel == 4 ? 108 : 40;

        // When we need to write out alpha data, the compression method BI_BITFIELDS (3) is used.
        // Otherwise, BI_RGB (0) is fine - no compression in other words.
        int compressionMethod = mBytesPerPixel == 4 ? 3 : 0;

        // BMP HEADER
        const int kBMPHeaderSize = 14;
        writer.WriteString("BM");
        writer.WriteUInt(0);    // Size of file in bytes. Optional to fill in.
        writer.WriteUShort(0);  // Reserved/empty
        writer.WriteUShort(0);  // Reserved/empty
        writer.WriteUInt(kBMPHeaderSize + dibHeaderSize);   // Offset to image data (from beginning of this file)

        // DIB HEADER
        writer.WriteUInt(dibHeaderSize);        // Size of DIB header. Indicates which header version is being used.
        writer.WriteInt(mWidth);                // Width of image; signed for some reason.
        writer.WriteInt(mHeight);               // Height of image; signed for some reason.
        writer.WriteUShort(1);                  // Number of color planes, always 1.

        uint16_t bitsPerPixel = (mPalette != nullptr ? 8 : mBytesPerPixel * 8);
        writer.WriteUShort(bitsPerPixel);                   // Number of bits-per-pixel.
        writer.WriteUInt(compressionMethod);                // Compression method.
        writer.WriteUInt(mWidth * mHeight * bitsPerPixel);  // Uncompressed size of image.
        writer.WriteInt(0);                                 // Preferred width for printing, unused.
        writer.WriteInt(0);                                 // Preferred height for printing, unused.
        writer.WriteUInt(0);                                // Number of palette colors, unused.
        writer.WriteUInt(0);                                // Number of important colors, unused.

        // If this image has alpha, and we're thus writing out a BITMAPV4HEADER DIB header, we need to write out some additional fields.
        if(mBytesPerPixel == 4)
        {
            // Define masks for each color component. This is just using the default used by BMP format anyway.
            writer.WriteUInt(0x00FF0000); // Red
            writer.WriteUInt(0x0000FF00); // Green
            writer.WriteUInt(0x000000FF); // Blue
            writer.WriteUInt(0xFF000000); // Alpha

            // We must write out a bunch of color space info, but it's mostly dummy/zeros.
            // Writing "Win " at the beginning uses the Windows color space.
            writer.WriteString("Win ");
            for(int i = 0; i < 12; ++i)
            {
                writer.WriteUInt(0);
            }
        }

        // COLOR TABLE - Only needed for 8BPP or less.
        if(bitsPerPixel <= 8)
        {
            writer.Write(mPalette, mPaletteSize);
        }

        // PIXELS
        // Write out one row at a time, bottom to top, left to right, per BMP format standard.
        int rowSize = CalculateBmpRowSize(bitsPerPixel, mWidth);
        for(int y = mHeight - 1; y >= 0; --y)
        {
            int bytesWritten = 0;
            for(uint32_t x = 0; x < mWidth; ++x)
            {
                if(bitsPerPixel == 8)
                {
                    writer.WriteByte(mPaletteIndexes[(y * mWidth + x)]);
                    ++bytesWritten;
                }
                else
                {
                    uint32_t index = (y * mWidth + x) * mBytesPerPixel;

                    if(mFormat == Format::BGR || mFormat == Format::BGRA)
                    {
                        writer.WriteByte(mPixels[index]);     // Blue
                        writer.WriteByte(mPixels[index + 1]); // Green
                        writer.WriteByte(mPixels[index + 2]); // Red
                    }
                    else
                    {
                        writer.WriteByte(mPixels[index + 2]); // Blue
                        writer.WriteByte(mPixels[index + 1]); // Green
                        writer.WriteByte(mPixels[index]);     // Red
                    }
                    bytesWritten += 3;

                    if(bitsPerPixel >= 32)
                    {
                        writer.WriteByte(mPixels[index + 3]); // Alpha
                        ++bytesWritten;
                    }
                }
            }

            // Add padding to write out total desired row size (padded to 4 bytes).
            while(bytesWritten < rowSize)
            {
                writer.WriteByte(0);
                bytesWritten++;
            }
        }
    }
}

void Texture::LoadInternal(BinaryReader& reader)
{
    // Texture can be in one of two formats:
    // 1) A custom/compressed format.
    // 2) A normal BMP format.
    // The first 2 byte value can tell us.
    unsigned short fileIdentifier = reader.ReadUShort();
    if(fileIdentifier == 0x3136) // 16
    {
        LoadCompressedFormat(reader);
    }
    else if(fileIdentifier == 0x4D42) // BM
    {
        LoadBmpFormat(reader);
    }
    else if(fileIdentifier == 0x5089)
    {
        // We read 2 bytes to identify this as a PNG file.
        // But then the PNG codec wants us to be at the start of the file. So...back up 2 bytes!
        reader.Seek(reader.GetPosition() - 2);
        LoadPngFormat(reader);
    }

    // This seeeeems to work consistently - if the top-left pixel is transparent or magenta, flag as alpha test.
    if(mHeight > 0 && mWidth > 0)
    {
        if((mBytesPerPixel == 4 && mPixels[3] == 0) ||
           (mBytesPerPixel == 3 && mPixels[0] == 255 && mPixels[2] == 255))
        {
            mRenderType = RenderType::AlphaTest;
        }
    }
}

void Texture::LoadCompressedFormat(BinaryReader& reader)
{
    // 2 bytes: compressed file identifier (assumed this has already been read in from constructor).
    // 2 bytes: The compressed format has a second value here.
    unsigned short fileIdentifier2 = reader.ReadUShort();
    if(fileIdentifier2 != 0x4D6E) // Mn
    {
        printf("BMP file does not have correct identifier!\n");
        return;
    }

    // Read width and height values.
    mHeight = reader.ReadUShort();
    mWidth = reader.ReadUShort();

    // All GK3 compressed images are 3BPP.
    // We could store internally as either RGB or BGR - whichever is most convenient.
    mBytesPerPixel = 3;
    mFormat = Format::RGB;

    // Allocate pixels array.
    mPixels = new uint8_t[mWidth * mHeight * mBytesPerPixel];

    // Read in pixel data.
    // This pixel data is stored top-left to bottom-right, so we don't flip (our pixel array starts at top-left corner).
    for(uint32_t y = 0; y < mHeight; ++y)
    {
        for(uint32_t x = 0; x < mWidth; ++x)
        {
            int current = (y  * mWidth + x) * mBytesPerPixel;
            uint16_t pixel = reader.ReadUShort();

            float red = static_cast<float>((pixel & 0xF800) >> 11);
            float green = static_cast<float>((pixel & 0x07E0) >> 5);
            float blue = static_cast<float>((pixel & 0x001F));

            mPixels[current] = (unsigned char)(red * 255 / 31);
            mPixels[current + 1] = (unsigned char)(green * 255 / 63);
            mPixels[current + 2] = (unsigned char)(blue * 255 / 31);

            /*
            // Causes all instances of magenta (R = 255, B = 255) to appear transparent.
            if(mPixels[current] > 200 && mPixels[current + 1] < 100 && mPixels[current + 2] > 200)
            {
                mPixels[current + 3] = 0;
            }
            else
            {
                mPixels[current + 3] = 255;
            }
            */
        }

        // Might need to skip some padding here.
        if((mWidth & 0x00000001) != 0)
        {
            reader.ReadUShort();
        }
    }
}

void Texture::LoadBmpFormat(BinaryReader& reader)
{
    // BMP HEADER
    // 2 bytes: BMP file identifier (assumed this has already been read in from constructor).
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
    uint16_t colorPlaneCount = reader.ReadUShort();
    if(colorPlaneCount != 1)
    {
        printf("Texture: unsupported color plane count: %u\n", colorPlaneCount);
        return;
    }

    // 2 bytes: number of bits per pixel
    uint16_t bitsPerPixel = reader.ReadUShort();
    mBytesPerPixel = bitsPerPixel / 8;
    if(bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32)
    {
        printf("Texture: unsupported bit depth: %u\n", bitsPerPixel);
        return;
    }

    // 4 bytes: compression method
    // 0 = BI_RGB (not compressed)
    // 1 = BI_RLE8 (RLE 8-bit/pixel)
    // 2 = BI_RLE4 (RLE 4-bit/pixel)
    // 3 = BI_BITFIELDS (???)
    // 4 = BI_JPEG (a JPEG image)
    // 5 = BI_PNG (a PNG image)
    // 6-13 = BI_ALPHABITFIELDS, BI_CMYK, BI_CMYKRLE8, BI_CMYKRLE4
    uint32_t compressionMethod = reader.ReadUInt();
    if(compressionMethod != 0)
    {
        printf("Texture: unsupported compression method: %u\n", compressionMethod);
        return;
    }

    // Figure out the pixel data format.
    if(bitsPerPixel == 8)
    {
        // For palettized textures, the format isn't in stone until we convert the palette data to pixel data.
        // But given that palette data is only RGB, the pixel data probably won't have alpha.
        mFormat = Format::BGR;
    }
    else if(bitsPerPixel == 24)
    {
        mFormat = Format::BGR;
    }
    else if(bitsPerPixel == 32)
    {
        mFormat = Format::BGRA;
    }

    // 4 bytes: uncompressed size; but if compression method is zero, this is usually also zero (unset).
    // 8 bytes: horizontal/vertical resolution (pixels per meter) - unused.
    reader.Skip(12);

    // 4 bytes: num colors in palette. If zero, default to 2^(bpp)
    uint32_t numColorsInColorPalette = reader.ReadUInt();
    if(numColorsInColorPalette == 0)
    {
        numColorsInColorPalette = Math::PowBase2(bitsPerPixel);
    }

    // 4 bytes: num important colors - unused.
    reader.Skip(4);

    // COLOR TABLE - only present for 8-bpp or lower images
    if(bitsPerPixel <= 8)
    {
        // The number of bytes is numColors in palette, times 4 bytes each.
        // The order of the colors is blue, green, red, alpha.
        mPaletteSize = numColorsInColorPalette * 4;
        mPalette = new uint8_t[mPaletteSize];
        reader.Read(mPalette, mPaletteSize);
    }

    // PIXELS
    // For 8-bpp images with a palette, allocate palette indexes.
    if(bitsPerPixel <= 8)
    {
        mPaletteIndexes = new uint8_t[mWidth * mHeight];
    }
    else
    {
        mPixels = new uint8_t[mWidth * mHeight * mBytesPerPixel];
    }

    // BMP files pad each row of pixels to a 4-byte alignment.
    // As a result, if a row of pixels isn't naturally 4-byte aligned (based on width and bytes per pixel), we will have padding bytes.
    int rowSize = CalculateBmpRowSize(bitsPerPixel, mWidth);
    bool hasPaddingBytes = (rowSize != mBytesPerPixel * mWidth);
    assert(rowSize >= mBytesPerPixel * mWidth);

    // As long as we don't have padding bytes, we can read in all pixel data very efficiently, with a single read operation.
    if(mPixels != nullptr && !hasPaddingBytes)
    {
        reader.Read(mPixels, mWidth * mHeight * mBytesPerPixel);

        // BMP data is bottom-left to top-right. But we want top-left to bottom-right.
        // Flipping the data vertically after reading in fixes this discrepancy.
        FlipVertically();
    }
    else // there are padding bytes, or this is a palettized image
    {
        // Because of padding bytes, we do the less efficient reading option.
        // BMP pixel data is stored bottom-left to top-right, so we do flip (our pixel array starts at top-left corner).
        int paddingByteCount = rowSize - (mBytesPerPixel * mWidth);
        for(int y = mHeight - 1; y >= 0; --y)
        {
            for(uint32_t x = 0; x < mWidth; ++x)
            {
                // Calculate index into pixels array.
                int index = (y * mWidth + x) * mBytesPerPixel;

                // How we interpret pixel data will depend on the bpp.
                if(bitsPerPixel == 8)
                {
                    // Read in the palette index and save it.
                    uint8_t paletteIndex = reader.ReadByte();
                    mPaletteIndexes[(y * mWidth + x)] = paletteIndex;
                }
                else if(bitsPerPixel == 24 || bitsPerPixel == 32)
                {
                    // Assuming BI_RGB format, alpha is not stored.
                    // So regardless of bits per pixel of 24 or 32, the data layout and size is the same.

                    // Pixel data in the BMP file is BGR.
                    mPixels[index] = reader.ReadByte();     // Blue
                    mPixels[index + 1] = reader.ReadByte(); // Green
                    mPixels[index + 2] = reader.ReadByte(); // Red

                    // BI_RGB format doesn't save any alpha, even if 32 bits per pixel.
                    // We'll use a placeholder of 255 (fully opaque).
                    if(bitsPerPixel == 32)
                    {
                        mPixels[index + 3] = 255; // Alpha
                    }
                }
            }

            // Skip padding that may be present, to ensure 4-byte alignment.
            reader.Skip(paddingByteCount);
        }
    }
}

void Texture::LoadPngFormat(BinaryReader& reader)
{
    PNG::ImageData imageData;
    PNG::CodecResult result = PNG::Decode(reader, imageData);
    if(result == PNG::CodecResult::Success)
    {
        mWidth = imageData.width;
        mHeight = imageData.height;
        mBytesPerPixel = imageData.bytesPerPixel;
        mFormat = Format::RGBA; // the decoder currently always decodes to RGBA format
        mPixels = imageData.pixelData;
    }
}

void Texture::CreatePixelsFromPaletteData()
{
    // If this is a palettized texture, but we want to render it or manipulate pixel data, we must generate the pixel data.
    if(mPixels == nullptr && mPalette != nullptr && mPaletteIndexes != nullptr)
    {
        // We'll assume palette data is in BGRA format, since that's what we read from the BMP file format.
        // Also, since BMP palette data doesn't actually store Alpha (it's placeholder), we can store as 24-bit.
        mBytesPerPixel = 3;
        mFormat = Format::BGR;

        // Allocate pixel array.
        int pixelCount = mWidth * mHeight;
        mPixels = new uint8_t[pixelCount * mBytesPerPixel];

        // Go pixel-by-pixel and fill in the pixels array by converting the palette to pixels.
        for(int pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
        {
            // Get the palette index this pixel uses and convert it to an offset within the palette index.
            // Remember, palette data is BGRA (even if A is unused), so we must multiply by 4 (for 4 bytes per pixel).
            int paletteByteIndex = mPaletteIndexes[pixelIndex] * 4;

            // Copy color from palette to the pixels array.
            int pixelsByteIndex = pixelIndex * mBytesPerPixel;
            mPixels[pixelsByteIndex] = mPalette[paletteByteIndex];
            mPixels[pixelsByteIndex + 1] = mPalette[paletteByteIndex + 1];
            mPixels[pixelsByteIndex + 2] = mPalette[paletteByteIndex + 2];
        }
    }
}
