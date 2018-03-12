//
// InputManager.h
//
// Clark Kromenaker
//
// Central spot that allows other systems to query for input events.
// Includes mouse, keyboard, gamepads, etc.
//
#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

class InputManager
{
public:
    InputManager();
    
    void Update();
    
    bool IsKeyDown(SDL_Scancode scancode);
    bool IsKeyPressed(SDL_Scancode scancode) { return mKeyboardState[scancode] == 1; }
    bool IsKeyUp(SDL_Scancode scancode);
    
    bool IsMouseButtonDown(int button);
    bool IsMouseButtonPressed(int button);
    bool IsMouseButtonUp(int button);
    
    Vector2 GetMousePosition() { return mMousePosition; }
    Vector2 GetMouseDelta() { return mMousePositionDelta; }
    
private:
    // KEYBOARD
    // Number of keys on the keyboard.
    int mNumKeys = 0;
    
    // A byte array where each byte indicates if a key is up or down.
    const uint8_t* mKeyboardState = nullptr;
    
    // The keyboard state from last frame, so we can check for up or down moments.
    uint8_t* mPrevKeyboardState = nullptr;
    
    // MOUSE
    // Current and previous mouse state, to detect button up and down events.
    Uint32 mMouseButtonState = 0;
    Uint32 mPrevMouseButtonState = 0;
    
    // The mouse's current position in window coords.
    Vector2 mMousePosition;
    
    // The mouse's position delta for this frame.
    Vector2 mMousePositionDelta;
};

inline bool InputManager::IsKeyDown(SDL_Scancode scancode)
{
    return mKeyboardState[scancode] == 1 && mPrevKeyboardState[scancode] == 0;
}

inline bool InputManager::IsKeyUp(SDL_Scancode scancode)
{
    return mKeyboardState[scancode] == 0 && mPrevKeyboardState[scancode] == 1;
}

inline bool InputManager::IsMouseButtonDown(int button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON(button));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON(button));
    return pressedThisFrame && !pressedLastFrame;
}

inline bool InputManager::IsMouseButtonPressed(int button)
{
    return (mMouseButtonState & SDL_BUTTON(button));
}

inline bool InputManager::IsMouseButtonUp(int button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON(button));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON(button));
    return !pressedThisFrame && pressedLastFrame;
}
