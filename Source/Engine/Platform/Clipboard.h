//
// Clark Kromenaker
//
// Interface for setting and getting clipboard text.
// (Originally thought this would be more difficult, but turns out SDL provides support for this whew).
//
#pragma once
#include <SDL.h>

namespace Clipboard
{
    inline void SetClipboardText(const char* text)
    {
        // Easy
        SDL_SetClipboardText(text);
    }

    inline const char* GetClipboardText()
    {
        // Peasy
        return SDL_GetClipboardText();
    }
}