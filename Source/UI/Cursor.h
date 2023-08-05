//
// Clark Kromenaker
//
// An asset that represents a mouse cursor.
//
#pragma once
#include "Asset.h"

#include <vector>

struct SDL_Cursor;

class Cursor : public Asset
{
public:
    Cursor(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~Cursor();

    void Load(uint8_t* data, uint32_t dataLength);
    
    void Activate(bool animate = true);
    
    void Update(float deltaTime);
    
private:
    // The frames making up the cursor.
    // For animated cursors, there may be multiple entries.
    std::vector<SDL_Cursor*> mCursorFrames;

    // If true, cursor will animate.
    // Sometimes, we want to purposely disable this.
    bool mAnimate = true;

    // For animated cursors, the speed of the animation.
    float mFramesPerSecond = 0.0f;

    // For animated cursors, the current frame index.
    float mFrameIndex = 0.0f;
};
