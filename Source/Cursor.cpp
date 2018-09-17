//
// Cursor.cpp
//
// Clark Kromenaker
//
#include "Cursor.h"

#include <SDL2/SDL.h>

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"
#include "Texture.h"

Cursor::Cursor(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

Cursor::~Cursor()
{
    for(auto& frame : mCursorFrames)
    {
        SDL_FreeCursor(frame);
    }
    mCursorFrames.clear();
}

void Cursor::Activate()
{
    if(mCursorFrames.size() > 0)
    {
        SDL_SetCursor(mCursorFrames[0]);
    }
    mFrameIndex = 0;
}

void Cursor::Update(float deltaTime)
{
    // Only need to update if the frame needs to change.
    if(mFrameCount < 2 || mFrameTime == 0.0f) { return; }
    
    mFrameTimer += deltaTime;
    if(mFrameTimer > mFrameTime)
    {
        mFrameTimer = 0.0f;
        
        mFrameIndex++;
        mFrameIndex %= mCursorFrames.size();
        
        SDL_SetCursor(mCursorFrames[mFrameIndex]);
    }
}

void Cursor::ParseFromData(char *data, int dataLength)
{
    // Texture used is always the same as the name of the cursor.
    Texture* texture = Services::GetAssets()->LoadTexture(GetNameNoExtension() + ".BMP");
    if(texture == nullptr)
    {
        std::cout << "Couldn't load texture for cursor " << mName << std::endl;
        return;
    }
    
    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);
    
    Vector2 hotspotVal;
    bool hotspotIsPercent = false;
    
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            IniKeyValue keyValue = parser.GetKeyValue();
            if(StringUtil::EqualsIgnoreCase(keyValue.key, "hotspot"))
            {
                if(StringUtil::EqualsIgnoreCase(keyValue.value, "center"))
                {
                    hotspotVal.SetX(0.5f);
                    hotspotVal.SetY(0.5f);
                    hotspotIsPercent = true;
                }
                else
                {
                    hotspotVal = keyValue.GetValueAsVector2();
                }
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame count"))
            {
                mFrameCount = keyValue.GetValueAsInt();
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "frame rate"))
            {
                int frameRate = keyValue.GetValueAsInt();
                mFrameTime = 1.0f / (float)frameRate;
            }
            else if(StringUtil::EqualsIgnoreCase(keyValue.key, "allow fading"))
            {
                mAllowFading = keyValue.GetValueAsBool();
            }
            // Function
            // Source Opacity
            // Destination Opacity
        }
    }
    
    SDL_Surface* srcSurface = texture->GetSurface();
    int frameWidth = texture->GetWidth() / mFrameCount;
    int frameHeight = texture->GetHeight();
    
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
    
    // Create cursors for each frame.
    for(int i = 0; i < mFrameCount; i++)
    {
        SDL_Rect srcRect;
        srcRect.x = i * frameWidth;
        srcRect.y = 0;
        srcRect.w = frameWidth;
        srcRect.h = frameHeight;
        
        SDL_Surface* dstSurface = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, rmask, gmask, bmask, amask);
        SDL_BlitSurface(srcSurface, &srcRect, dstSurface, NULL);
        
        Vector2 hotspot = hotspotVal;
        if(hotspotIsPercent)
        {
            hotspot.SetX((texture->GetWidth() / mFrameCount) * hotspotVal.GetX());
            hotspot.SetY(texture->GetHeight() * hotspotVal.GetY());
        }
        
        SDL_Cursor* cursor = SDL_CreateColorCursor(dstSurface, (int)hotspot.GetX(), (int)hotspot.GetY());
        if(cursor == nullptr)
        {
            std::cout << "Create cursor failed: " << SDL_GetError() << std::endl;
        }
        mCursorFrames.push_back(cursor);
    }
}
