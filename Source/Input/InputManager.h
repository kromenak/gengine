//
// InputManager.h
//
// Clark Kromenaker
//
// Central spot that allows other systems to query for input events.
// Includes mouse, keyboard, gamepads, etc.
//
#pragma once
#include <string>

#include <SDL2/SDL.h>

#include "Vector2.h"

class TextInput;

class InputManager
{
public:
    enum class MouseButton
    {
        Left = 1,
        Middle,
        Right
    };
    
    InputManager();
    
    void Update();
    
    bool IsKeyDown(SDL_Scancode scancode);
    bool IsKeyPressed(SDL_Scancode scancode) { return mKeyboardState[scancode] == 1; }
    bool IsKeyUp(SDL_Scancode scancode);
    
    bool IsMouseButtonDown(MouseButton button);
    bool IsMouseButtonPressed(MouseButton button);
    bool IsMouseButtonUp(MouseButton button);
    
    Vector2 GetMousePosition() { return mMousePosition; }
    Vector2 GetMouseDelta() { return mMousePositionDelta; }
	
	void LockMouse();
	void UnlockMouse();
	bool MouseLocked() const { return mMouseLocked; }
	
	// Text input support
	void StartTextInput(TextInput* textInput);
	void StopTextInput();
	bool IsTextInput() const { return mTextInput != nullptr; }
	TextInput* GetTextInput() { return mTextInput; }
	
private:
    // KEYBOARD
    // Number of keys on the keyboard.
    int mNumKeys = 0;
    
    // A byte array where each byte indicates if a key is up or down.
	// Current and previous states, so we can check for up or down moments.
    const uint8_t* mKeyboardState = nullptr;
    uint8_t* mPrevKeyboardState = nullptr;
    
    // MOUSE
    // Current and previous mouse state, to detect button up and down events.
    uint32_t mMouseButtonState = 0;
    uint32_t mPrevMouseButtonState = 0;
    
    // The mouse's current position in window coords.
    Vector2 mMousePosition;
    
    // The mouse's position delta for this frame.
    Vector2 mMousePositionDelta;
		
	// Is the mouse locked?
	bool mMouseLocked = false;
	Vector2 mLockedMousePosition;
	
	// TEXT INPUT
	TextInput* mTextInput = nullptr;
};

inline bool InputManager::IsKeyDown(SDL_Scancode scancode)
{
    return mKeyboardState[scancode] == 1 && mPrevKeyboardState[scancode] == 0;
}

inline bool InputManager::IsKeyUp(SDL_Scancode scancode)
{
    return mKeyboardState[scancode] == 0 && mPrevKeyboardState[scancode] == 1;
}

inline bool InputManager::IsMouseButtonDown(MouseButton button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON((int)button));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON((int)button));
    return pressedThisFrame && !pressedLastFrame;
}

inline bool InputManager::IsMouseButtonPressed(MouseButton button)
{
    return (mMouseButtonState & SDL_BUTTON((int)button));
}

inline bool InputManager::IsMouseButtonUp(MouseButton button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON((int)button));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON((int)button));
    return !pressedThisFrame && pressedLastFrame;
}
