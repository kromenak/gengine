#include "Cursor.h"

#include <memory> // std::unique_ptr

#include <SDL.h>
#include <stb_image_resize.h>

#include "AssetManager.h"
#include "GKPrefs.h"
#include "IniParser.h"
#include "StringUtil.h"
#include "Texture.h"
#include "UIUtil.h"

TYPEINFO_INIT(Cursor, Asset, GENERATE_TYPE_ID)
{
    TYPEINFO_VAR(Cursor, VariableType::Bool, mAnimate);
    TYPEINFO_VAR(Cursor, VariableType::Int, mFramesPerSecond);
}

Cursor::~Cursor()
{
    FreeCursorFrames();
}

void Cursor::Load(uint8_t* data, uint32_t dataLength)
{
    // Texture used is always the same as the name of the cursor.
    mTexture = gAssetManager.LoadTexture(GetNameNoExtension(), GetScope());
    if(mTexture == nullptr)
    {
        printf("Create cursor %s failed: couldn't load texture.\n", mName.c_str());
        return;
    }

    // Parse data from ini format.
    bool hotspotIsPercent = false;
    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            const IniKeyValue& keyValue = parser.GetKeyValue();
            if(StringUtil::EqualsIgnoreCase(keyValue.key, "hotspot"))
            {
                if(StringUtil::EqualsIgnoreCase(keyValue.value, "center"))
                {
                    mHotspot.x = 0.5f;
                    mHotspot.y = 0.5f;
                    hotspotIsPercent = true;
                }
                else
                {
                    mHotspot = keyValue.GetValueAsVector2();
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame count"))
            {
                mFrameCount = Math::Max(keyValue.GetValueAsInt(), 1);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame rate"))
            {
                mFramesPerSecond = static_cast<float>(keyValue.GetValueAsInt());
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "allow fading"))
            {
                //TODO
                //keyValue.GetValueAsBool();
            }
            // Function
            // Source Opacity
            // Destination Opacity
        }
    }

    // If hotspot was a percent, convert to pixel position.
    if(hotspotIsPercent)
    {
        uint32_t frameWidth = mTexture->GetWidth() / mFrameCount;
        uint32_t frameHeight = mTexture->GetHeight();
        mHotspot.x = frameWidth * mHotspot.x;
        mHotspot.y = frameHeight * mHotspot.y;
    }

    // Convert the texture data into individual SDL cursor frames.
    RecreateCursorFramesIfNeeded();
}

void Cursor::Activate(bool animate)
{
    // See if this cursor needs its frames recreated (due to scale factor change).
    RecreateCursorFramesIfNeeded();

    // Set to first frame.
    if(!mCursorFrames.empty())
    {
        SDL_SetCursor(mCursorFrames[0]);
    }
    mFrameIndex = 0.0f;

    // Save animation pref.
    mAnimate = animate;
}

void Cursor::Update(float deltaTime)
{
    // See if this cursor needs its frames recreated (due to scale factor change).
    if(RecreateCursorFramesIfNeeded())
    {
        SDL_SetCursor(mCursorFrames[static_cast<int>(mFrameIndex)]);
    }

    // Only need to update if there are multiple frames to animate.
    if(!mAnimate || mCursorFrames.size() < 2) { return; }

    // Increase timer, but keep in bounds.
    mFrameIndex += mFramesPerSecond * deltaTime;
    while(mFrameIndex >= mCursorFrames.size())
    {
        mFrameIndex -= mCursorFrames.size();
    }

    // Set frame.
    SDL_SetCursor(mCursorFrames[static_cast<int>(mFrameIndex)]);
}

bool Cursor::RecreateCursorFramesIfNeeded()
{
    float scaleFactor = UI::GetScaleFactor(Prefs::GetMinimumScaleUIHeight(), Prefs::UsePixelPerfectUIScaling(), Prefs::GetUIScalingBias());
    if(!Math::AreEqual(mScaleFactor, scaleFactor))
    {
        CreateCursorFrames(scaleFactor);
        return true;
    }
    return false;
}

void Cursor::CreateCursorFrames(float scaleFactor)
{
    // Make sure any previously allocated cursor frames are freed.
    FreeCursorFrames();

    // Generate RGB masks (taken straight from SDL docs).
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

    // Get cursor texture info.
    uint32_t texBytesPerPixel = mTexture->GetBytesPerPixel();
    uint32_t texWidth = mTexture->GetWidth();
    uint32_t texHeight = mTexture->GetHeight();
    uint8_t* texPixels = mTexture->GetPixelData();

    // Figure out hotspot pixel.
    int hotspotX = static_cast<int>(mHotspot.x);
    int hotspotY = static_cast<int>(mHotspot.y);

    // If scaling the texture, we need to duplicate the texture pixels and resize.
    std::unique_ptr<uint8_t[]> allocatedPixelData = nullptr;
    if(!Math::AreEqual(scaleFactor, 1.0f))
    {
        // Width and height are increased by scale factor.
        texWidth *= scaleFactor;
        texHeight *= scaleFactor;

        // Make a new set of pixels at the desired size.
        allocatedPixelData = std::make_unique<uint8_t[]>(texWidth * texHeight * texBytesPerPixel);
        texPixels = allocatedPixelData.get();

        // Resize original pixels into new pixel buffer.
        // Using FILTER_BOX avoids color interpolation - needed for color key transparency to work correctly.
        stbir_resize_uint8_generic(mTexture->GetPixelData(), mTexture->GetWidth(), mTexture->GetHeight(), 0,
                                   texPixels, texWidth, texHeight, 0, texBytesPerPixel, -1, 0,
                                   STBIR_EDGE_CLAMP, STBIR_FILTER_BOX, STBIR_COLORSPACE_LINEAR, nullptr);

        // Hotspot also adjusts by scale factor.
        hotspotX *= scaleFactor;
        hotspotY *= scaleFactor;

        /*
        stbir_resize_uint8(mTexture->GetPixelData(), mTexture->GetWidth(), mTexture->GetHeight(), 0,
                           texPixels, texWidth, texHeight, 0, texBytesPerPixel);
        */
    }
    // BELOW HERE: DO NOT USE mTexture! Use local vars only (texPixels, texWidth, texHeight, etc).

    // Create a surface from the texture pixels.
    Uint32 textureBitsPerPixel = texBytesPerPixel * 8;
    Uint32 texturePitch = texBytesPerPixel * texWidth;
    SDL_Surface* srcSurface = SDL_CreateRGBSurfaceFrom(texPixels, texWidth, texHeight,
                                                       textureBitsPerPixel,
                                                       texturePitch,
                                                       rmask, gmask, bmask, amask);
    if(srcSurface == nullptr)
    {
        printf("Create cursor %s failed: couldn't create surface from texture (%s).\n", mName.c_str(), SDL_GetError());
        return;
    }

    // If texture is 3BPP, it won't have an alpha channel. But such textures CAN still have transparent pixels.
    // In this case, when the source surface is blitted to the destination surface, magenta will be treated as transparent.
    if(texBytesPerPixel < 4)
    {
        Uint32 key = SDL_MapRGB(srcSurface->format, 255, 0, 255); // Magenta
        SDL_SetColorKey(srcSurface, SDL_TRUE, key);
    }

    // Create cursors for each frame.
    int frameWidth = texWidth / mFrameCount;
    int frameHeight = texHeight;
    for(int i = 0; i < mFrameCount; ++i)
    {
        // Cursor frames are all in a single horizontal row.
        // So x varies by frame index, y is always 0. Width/height are also constant.
        SDL_Rect srcRect;
        srcRect.x = i * frameWidth;
        srcRect.y = 0;
        srcRect.w = frameWidth;
        srcRect.h = frameHeight;

        // Copy frame from source surface into a destination surface.
        // This destination surface should always be 32-bit (RGBA) so it can have transparent pixels.
        SDL_Surface* dstSurface = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, rmask, gmask, bmask, amask);
        if(dstSurface == nullptr)
        {
            printf("Create cursor %s failed: couldn't create dest surface for frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }
        int result = SDL_BlitSurface(srcSurface, &srcRect, dstSurface, nullptr);
        if(result != 0)
        {
            printf("Create cursor %s failed: couldn't blit to dest surface for frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }

        // Use destination surface to create cursor. This duplicates the surface pixel data.
        SDL_Cursor* cursor = SDL_CreateColorCursor(dstSurface, hotspotX, hotspotY);
        if(cursor == nullptr)
        {
            printf("Create cursor %s failed: couldn't create cursor frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }
        mCursorFrames.push_back(cursor);

        // Once the cursor is created, the destination surface is no longer needed.
        SDL_FreeSurface(dstSurface);
    }

    // The source surface is only needed to blit the individual cursor frames. We can free it when done with that.
    SDL_FreeSurface(srcSurface);

    // Save the used scale factor.
    mScaleFactor = scaleFactor;
}

void Cursor::FreeCursorFrames()
{
    for(auto& frame : mCursorFrames)
    {
        SDL_FreeCursor(frame);
    }
    mCursorFrames.clear();
}