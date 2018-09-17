//
// InputManager.cpp
//
// Clark Kromenaker
//
#include "InputManager.h"

InputManager::InputManager()
{
    // Returns chunk of memory owned by SDL, which contains the keyboard state.
    // We don't even need to update this, since it's a pointer. SDL does it for us.
    mKeyboardState = SDL_GetKeyboardState(&mNumKeys);
    
    // Allocate previous keyboard state array.
    mPrevKeyboardState = new uint8_t[mNumKeys];
}

void InputManager::Update()
{
    // Copy previous keyboard state each frame.
    SDL_memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);
    
    // Copy previous mouse state each frame.
    mPrevMouseButtonState = mMouseButtonState;
    
    // This queries device state from the OS. Marks switch from
    // "last frame values" to "current frame values".
    SDL_PumpEvents();
    
    // Query the mouse state.
    int x = 0;
    int y = 0;
    mMouseButtonState = SDL_GetMouseState(&x, &y);
    
    // Calculate mouse delta since last frame.
    int deltaX = x - mMousePosition.GetX();
    int deltaY = y - mMousePosition.GetY();
    mMousePositionDelta.SetX(deltaX);
    mMousePositionDelta.SetY(deltaY);
    
    // After delta calc, set mouse position.
    mMousePosition.SetX(x);
    mMousePosition.SetY(y);
}

void InputManager::DispatchEvents()
{
    
}
