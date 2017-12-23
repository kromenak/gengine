//
// InputManager.h
//
// Clark Kromenaker
//
// Central spot that allows other systems to query for input events.
// Includes mouse, keyboard, gamepads, etc.
//
#pragma once
#include "SDL/SDL.h"

class InputManager
{
public:
    InputManager();
    
    void Update();
    
    bool IsPressed(SDL_Scancode scancode) { return mKeyboardState[scancode] == 1; }
    
private:
    int mNumKeys = 0;
    const uint8_t* mKeyboardState = nullptr;
    uint8_t* mPrevKeyboardState = nullptr;
};
