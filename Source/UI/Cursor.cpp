#include "Cursor.h"

#include <SDL.h>

#include "AssetManager.h"
#include "IniParser.h"
#include "StringUtil.h"
#include "Texture.h"

Cursor::~Cursor()
{
    for(auto& frame : mCursorFrames)
    {
        SDL_FreeCursor(frame);
    }
}

void Cursor::Load(uint8_t* data, uint32_t dataLength)
{
    // Texture used is always the same as the name of the cursor.
    Texture* texture = gAssetManager.LoadTexture(GetNameNoExtension(), GetScope());
    if(texture == nullptr)
    {
        printf("Create cursor %s failed: couldn't load texture.\n", mName.c_str());
        return;
    }

    // Parse data from ini format.
    Vector2 hotspot;
    bool hotspotIsPercent = false;
    int frameCount = 1;

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
                    hotspot.x = 0.5f;
                    hotspot.y = 0.5f;
                    hotspotIsPercent = true;
                }
                else
                {
                    hotspot = keyValue.GetValueAsVector2();
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame count"))
            {
                frameCount = keyValue.GetValueAsInt();
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

    // Determine width/height of each cursor animation frame.
    // If cursor has multiple frames, they are laid out horizontally and all equal size.
    int frameWidth = texture->GetWidth() / frameCount;
    int frameHeight = texture->GetHeight();

    // If hotspot was a percent, convert to pixel position, now that we know the frame width/height.
    if(hotspotIsPercent)
    {
        hotspot.x = frameWidth * hotspot.x;
        hotspot.y = frameHeight * hotspot.y;
    }

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

    // Create a surface from the texture.
    const Uint32 kTextureBytesPerPixel = 4;
    const Uint32 kTextureBitsPerPixel = kTextureBytesPerPixel * 8;
    Uint32 texturePitch = kTextureBytesPerPixel * texture->GetWidth();
    SDL_Surface* srcSurface = SDL_CreateRGBSurfaceFrom(texture->GetPixelData(), texture->GetWidth(), texture->GetHeight(),
                                                       kTextureBitsPerPixel,
                                                       texturePitch,
                                                       rmask, gmask, bmask, amask);
    if(srcSurface == nullptr)
    {
        printf("Create cursor %s failed: couldn't create surface from texture (%s).\n", mName.c_str(), SDL_GetError());
        return;
    }

    // Create cursors for each frame.
    for(int i = 0; i < frameCount; i++)
    {
        SDL_Rect srcRect;
        srcRect.x = i * frameWidth;
        srcRect.y = 0;
        srcRect.w = frameWidth;
        srcRect.h = frameHeight;

        // Copy frame from texture into a destination surface.
        SDL_Surface* dstSurface = SDL_CreateRGBSurface(0, frameWidth, frameHeight, kTextureBitsPerPixel, rmask, gmask, bmask, amask);
        if(dstSurface == nullptr)
        {
            printf("Create cursor %s failed: couldn't create dest surface for frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }
        int result = SDL_BlitSurface(srcSurface, &srcRect, dstSurface, nullptr);
        if(result != 0)
        {
            printf("Create cursor %s failed: couldn't blit to dest surface for frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }

        // Use destination surface to create cursor.
        SDL_Cursor* cursor = SDL_CreateColorCursor(dstSurface, (int)hotspot.x, (int)hotspot.y);
        if(cursor == nullptr)
        {
            printf("Create cursor %s failed: couldn't create cursor frame %i (%s).\n", mName.c_str(), i, SDL_GetError());
        }
        mCursorFrames.push_back(cursor);
    }
}

void Cursor::Activate(bool animate)
{
    // Set to first frame.
    if(mCursorFrames.size() > 0)
    {
        SDL_SetCursor(mCursorFrames[0]);
    }
    mFrameIndex = 0.0f;

    // Save animation pref.
    mAnimate = animate;
}

void Cursor::Update(float deltaTime)
{
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
