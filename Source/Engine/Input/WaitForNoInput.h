//
// Clark Kromenaker
//
// A utility that checks for inputs, but only after detecting no inputs for at least one frame.
//
// This can be useful to avoid accidental key press detection when transitioning between screens,
// especially with unclear/indeterminant update order issues.
//
// An example use case: A video is playing and the user presses ESC to skip it.
// The video end callback shows another screen, which also checks for ESC input as a shortcut to "continue" or similar.
// The other screen updates on the same frame, captures/uses the same ESC input, and accidentally skips the entire screen immediately.
//
// This class says "if the button was pressed, but only after detecting no input for at least one frame."
//
#pragma once
#include "InputManager.h"

class WaitForNoInput
{
public:
    void Reset()
    {
        mWaitingForNoInput = true;
    }

    bool IsAnyKeyLeadingEdge()
    {
        if(!WaitingForNoInput())
        {
            return gInputManager.IsAnyKeyLeadingEdge();
        }
        return false;
    }

    bool IsAnyMouseButtonLeadingEdge()
    {
        if(!WaitingForNoInput())
        {
            return gInputManager.IsAnyMouseButtonLeadingEdge();
        }
        return false;
    }

    bool IsKeyLeadingEdge(SDL_Scancode scancode)
    {
        if(!WaitingForNoInput())
        {
            return gInputManager.IsKeyLeadingEdge(scancode);
        }
        return false;
    }

private:
    // No input will be detected via this class until this is false.
    bool mWaitingForNoInput = true;

    bool WaitingForNoInput()
    {
        // Only set the flag to false if we detect no input for at least one frame.
        if(mWaitingForNoInput)
        {
            bool anyInput = gInputManager.IsAnyKeyPressed() || gInputManager.IsAnyMouseButtonPressed();
            if(!anyInput)
            {
                mWaitingForNoInput = false;
            }
        }
        return mWaitingForNoInput;
    }
};