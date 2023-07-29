#include "InputManager.h"

#include "Tools.h"
#include "UICanvas.h"
#include "Window.h"

InputManager gInputManager;

InputManager::~InputManager()
{
    delete[] mPrevKeyboardState;
}

void InputManager::Init()
{
    // Retrieve SDL keyboard state and number of keys.
    mKeyboardState = SDL_GetKeyboardState(&mNumKeys);

    // Use returned keyboard state as our current keyboard state.
    mCurrKeyboardState = mKeyboardState;

    // Allocate previous keyboard state array.
    mPrevKeyboardState = new uint8_t[mNumKeys];
}

void InputManager::Update()
{
    // Copy previous keyboard state each frame.
    // Don't do this if a tool has keyboard focus so the game ignores inputs during that time.
    if(!Tools::EatingKeyboardInputs())
    {
        SDL_memcpy(mPrevKeyboardState, mCurrKeyboardState, mNumKeys);
    }
    
    // Copy previous mouse state each frame.
    mPrevMouseButtonState = mMouseButtonState;

    // Clear previous frame's mouse wheel scroll.
    mMouseWheelScrollDelta = Vector2::Zero;
    
    // This queries device state from the OS.
    // Marks switch from "last frame values" to "current frame values".
    SDL_PumpEvents();

    // Copy current keyboard state over.
    // If the tool is eating inputs, we just "reuse" prev keyboard state until the tool is done.
    // Again, this just stops the game from using inputs meant for the tool.
    mCurrKeyboardState = (Tools::EatingKeyboardInputs() ? mPrevKeyboardState : mKeyboardState);

    // Update mouse state. This differs base on whether a tool is using mouse inputs.
    if(!Tools::EatingMouseInputs())
    {
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
            mouseY = Window::GetHeight() - mouseY;

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
    else // a tool overlay is using the mouse
    {
        // Resetting all mouse state in this case seems to work fine...
        mPrevMouseButtonState = 0;
        mMouseButtonState = 0;

        mMousePosition = Vector2::Zero;
        mMousePositionDelta = Vector2::Zero;

        mMouseLocked = false;
        mLockedMousePosition = Vector2::Zero;
    }

    // Tools always require SDL "text input" mode to be active (or else keyboard inputs don't work in the tools).
    // But if a tool is not active, stop "text input" mode when no text input is active.
    if(Tools::EatingKeyboardInputs())
    {
        if(!SDL_IsTextInputActive())
        {
            SDL_StartTextInput();
        }
    }
    else if(mTextInput == nullptr && SDL_IsTextInputActive())
    {
        SDL_StopTextInput();
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
