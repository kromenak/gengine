//
// Cursor.h
//
// Clark Kromenaker
//
// An asset that represents a mouse cursor in the game.
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
    
    void Activate();
    
    void Update(float deltaTime);
    
private:
    Vector2 mHotspot;
    int mFrameCount = 1;
    float mFrameTime = 0.1f;
    bool mAllowFading = true;
    
    int mFrameIndex = 0;
    float mFrameTimer = 0.0f;
    
    std::vector<SDL_Cursor*> mCursorFrames;
    
    void ParseFromData(char* data, int dataLength);
};
