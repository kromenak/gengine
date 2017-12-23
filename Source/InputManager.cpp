//
// InputManager.cpp
//
// Clark Kromenaker
//
#include "InputManager.h"

InputManager::InputManager()
{
    // Allocate previous keyboard state array.
    mKeyboardState = SDL_GetKeyboardState(&mNumKeys);
    mPrevKeyboardState = new uint8_t[mNumKeys];
}

void InputManager::Update()
{
    SDL_memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);
}
