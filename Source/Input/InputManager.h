//
// Clark Kromenaker
//
// Central spot that allows other systems to query for input events.
// Includes mouse, keyboard, gamepads, etc.
//
#pragma once
#include <string>

#include <SDL.h>

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
    ~InputManager();
    
    void Update();

    // Keyboard
    bool IsKeyLeadingEdge(SDL_Scancode scancode);
    bool IsKeyPressed(SDL_Scancode scancode) { return mCurrKeyboardState[scancode] == 1; }
    bool IsKeyTrailingEdge(SDL_Scancode scancode);

    bool IsAnyKeyLeadingEdge();
    bool IsAnyKeyPressed();

    // Mouse
    bool IsMouseButtonLeadingEdge(MouseButton button);
    bool IsMouseButtonPressed(MouseButton button) { return (mMouseButtonState & SDL_BUTTON(static_cast<int>(button))); }
    bool IsMouseButtonTrailingEdge(MouseButton button);
    
    Vector2 GetMousePosition() { return mMousePosition; }
    Vector2 GetMouseDelta() { return mMousePositionDelta; }

    void OnMouseWheelScroll(const Vector2& scroll) { mMouseWheelScrollDelta = scroll; }
    const Vector2& GetMouseWheelScrollDelta() const { return mMouseWheelScrollDelta; }

	void LockMouse();
	void UnlockMouse();
	bool MouseLocked() const { return mMouseLocked; }
	
	// Text Input
	void StartTextInput(TextInput* textInput);
	void StopTextInput();
	bool IsTextInput() const { return mTextInput != nullptr; }
	TextInput* GetTextInput() { return mTextInput; }
	
private:
    // KEYBOARD
    // Number of keys on the keyboard.
    int mNumKeys = 0;

    // Pointer to keyboard state reported by SDL.
    // SDL owns this memory, and it's updated automatically when SDL_PumpEvents is called.
    const uint8_t* mKeyboardState = nullptr;

    // A byte array where each byte indicates if a key is up or down.
	// Current and previous states, so we can check for up or down moments.
    const uint8_t* mCurrKeyboardState = nullptr;
    uint8_t* mPrevKeyboardState = nullptr;
    
    // MOUSE
    // Current and previous mouse state, to detect button up and down events.
    uint32_t mMouseButtonState = 0;
    uint32_t mPrevMouseButtonState = 0;
    
    // The mouse's current position in window coords.
    Vector2 mMousePosition;
    
    // The mouse's position delta for this frame.
    Vector2 mMousePositionDelta;

    // Amount of scrolling on the mouse wheel detected this frame.
    Vector2 mMouseWheelScrollDelta;
		
	// Is the mouse locked?
	bool mMouseLocked = false;
	Vector2 mLockedMousePosition;
	
	// TEXT INPUT
	TextInput* mTextInput = nullptr;
};

inline bool InputManager::IsKeyLeadingEdge(SDL_Scancode scancode)
{
    return mCurrKeyboardState[scancode] == 1 && mPrevKeyboardState[scancode] == 0;
}

inline bool InputManager::IsKeyTrailingEdge(SDL_Scancode scancode)
{
    return mCurrKeyboardState[scancode] == 0 && mPrevKeyboardState[scancode] == 1;
}

inline bool InputManager::IsAnyKeyLeadingEdge()
{
    //TODO: Any faster way to do this?
    for(int i = 0; i < mNumKeys; ++i)
    {
        if(mCurrKeyboardState[i] == 1 && mPrevKeyboardState[i] == 0) { return true; }
    }
    return false;
}

inline bool InputManager::IsAnyKeyPressed()
{
    //TODO: Any faster way to do this?
    for(int i = 0; i < mNumKeys; ++i)
    {
        if(mCurrKeyboardState[i] != 0) { return true; }
    }
    return false;
}

inline bool InputManager::IsMouseButtonLeadingEdge(MouseButton button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON(static_cast<int>(button)));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON(static_cast<int>(button)));
    return pressedThisFrame && !pressedLastFrame;
}

inline bool InputManager::IsMouseButtonTrailingEdge(MouseButton button)
{
    bool pressedThisFrame = (mMouseButtonState & SDL_BUTTON(static_cast<int>(button)));
    bool pressedLastFrame = (mPrevMouseButtonState & SDL_BUTTON(static_cast<int>(button)));
    return !pressedThisFrame && pressedLastFrame;
}
