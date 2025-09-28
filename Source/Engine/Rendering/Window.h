//
// Clark Kromenaker
//
// Cross-platform helper for creating and manipulating the window.
//
// The window is rendering-adjacent, but it is usually just an OS construct
// upon which the rendering surface is placed. Better to keep it separated.
//
#pragma once
#include <cstdint>
#include <vector>

#include <SDL.h>

#include "Rect.h"
#include "Vector2.h"

namespace Window
{
    struct Resolution
    {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    enum class Mode
    {
        Windowed,               // Game is in a floating window on the desktop.
        FullscreenExclusive,    // Game is fullscreen, in exclusive mode.
        FullscreenDesktop       // Game is fullscreen, but just as a window at the desired fullscreen size.
    };

    void Create(const char* title);
    void Create(const char* title, int x, int y, int w, int h, Uint32 flags);
    void Destroy();

    SDL_Window* Get();

    // Fullscreen/Windowed
    void SetFullscreen(bool fullscreen);
    bool IsFullscreen();
    void ToggleFullscreen();
    Mode GetFullscreenMode();

    // Resolution & Size
    const std::vector<Resolution>& GetResolutions();
    const Resolution& GetResolution();
    void SetResolution(const Resolution& resolution);

    inline uint32_t GetWidth() { return GetResolution().width; }
    inline uint32_t GetHeight() { return GetResolution().height; }
    inline Vector2 GetSize() { return Vector2(static_cast<float>(GetWidth()), static_cast<float>(GetHeight())); }
    inline Rect GetRect() { return Rect(0, 0, static_cast<float>(GetWidth()), static_cast<float>(GetHeight())); }

    // Position
    Vector2 GetPosition();
    void SetPosition(int x, int y);
    void OnPositionChanged();

    // Title
    void SetTitle(const char* title);
    const char* GetTitle();
};