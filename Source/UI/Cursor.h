//
// Clark Kromenaker
//
// An asset that represents a mouse cursor.
//
#pragma once
#include "Asset.h"

#include <vector>

#include "Vector2.h"

struct SDL_Cursor;

class Cursor : public Asset
{
public:
    Cursor(std::string name, char* data, int dataLength);
    ~Cursor();
    
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
    
    void ParseFromData(char* data, int dataLength);
};
