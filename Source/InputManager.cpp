//
// InputManager.cpp
//
// Clark Kromenaker
//
#include "InputManager.h"

#include "Services.h"

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
	
	// INVERT the mouse Y.
	// SDL returns mouse coords from top-left, but we want it from bottom-left.
	mouseY = Services::GetRenderer()->GetWindowHeight() - mouseY;
	
    // Calculate mouse delta since last frame.
    int deltaX = mouseX - mMousePosition.GetX();
    int deltaY = mouseY - mMousePosition.GetY();
    mMousePositionDelta.SetX(deltaX);
    mMousePositionDelta.SetY(deltaY);
    
    // After delta calc, set mouse position.
    mMousePosition.SetX(mouseX);
    mMousePosition.SetY(mouseY);
}

void InputManager::StartListenForKeyboardInput(std::string initialText)
{
	SDL_StartTextInput();
	mTextInput = initialText;
	mIsTextInput = true;
}

void InputManager::StopListenForKeyboardInput()
{
	SDL_StopTextInput();
	mIsTextInput = false;
}

void InputManager::AppendText(std::string text)
{
	mTextInput += text;
}

void InputManager::Backspace()
{
	if(mTextInput.size() > 0)
	{
		mTextInput.pop_back();
	}
}

std::string& InputManager::GetTextInput()
{
	return mTextInput;
}
