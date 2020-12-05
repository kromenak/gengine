//
// InputManager.cpp
//
// Clark Kromenaker
//
#include "InputManager.h"

#include "Services.h"
#include "UICanvas.h"

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
	// This gives us button press data AND mouse position.
    int mouseX = 0;
    int mouseY = 0;
    mMouseButtonState = SDL_GetMouseState(&mouseX, &mouseY);
	
    // When mouse-locked, we only track deltas.
	if(mMouseLocked)
	{
		// Use relative query when mouse is locked.
		int deltaX = 0;
		int deltaY = 0;
		SDL_GetRelativeMouseState(&deltaX, &deltaY);
		mMousePositionDelta.x = deltaX;
		mMousePositionDelta.y = -deltaY; // negate b/c we treat +y is up.
		
		// Though SDL may be "under the hood" moving the mouse around in locked mode,
		// we don't update our mouse position. To us, the mouse is not moving until locked mode ends.
	}
	else
	{
		// INVERT the mouse Y.
		// SDL returns mouse coords from top-left, but we want it from bottom-left.
		mouseY = Services::GetRenderer()->GetWindowHeight() - mouseY;
		
		// Calculate delta from last mouse position.
		int deltaX = static_cast<int>(mouseX - mMousePosition.x);
		int deltaY = static_cast<int>(mouseY - mMousePosition.y);
		mMousePositionDelta.x = deltaX;
		mMousePositionDelta.y = deltaY;
		
		// After delta calc, set mouse position.
		mMousePosition.x = mouseX;
		mMousePosition.y = mouseY;
		
		// Handle UI input.
		UICanvas::UpdateInput();
	}
}

void InputManager::LockMouse()
{
	if(!mMouseLocked)
	{
		// Save position of mouse when we locked it.
		int x = 0;
		int y = 0;
		SDL_GetMouseState(&x, &y);
		mLockedMousePosition.x = x;
		mLockedMousePosition.y = y;
		
		// Enable relative mode.
		// Call GetState once to clear any store deltas.
		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_GetRelativeMouseState(nullptr, nullptr);
		
		// Mouse is locked!
		mMouseLocked = true;
	}
}

void InputManager::UnlockMouse()
{
	if(mMouseLocked)
	{
		// Disable relative mode.
		SDL_SetRelativeMouseMode(SDL_FALSE);
		
		// Move mouse back to position it was locked at.
		// Ensures mouse cursor position doesn't change from when we entered mouse lock to left it.
		SDL_WarpMouseInWindow(nullptr, mLockedMousePosition.x, mLockedMousePosition.y);
		
		// No longer locked.
		mMouseLocked = false;
	}
}

void InputManager::StartTextInput(TextInput* textInput)
{
	SDL_StartTextInput();
	mTextInput = textInput;
}

void InputManager::StopTextInput()
{
	SDL_StopTextInput();
	mTextInput = nullptr;
}
