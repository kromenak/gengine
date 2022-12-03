#include "Window.h"

#include <cassert>
#include <vector>

#include "SaveManager.h"
#include "StringUtil.h"

namespace Window
{
    // The game window. We only support one!
    SDL_Window* window = nullptr;
    
    // Default window width & height, if no other preference is specified.
    const int kDefaultWidth = 640;
    const int kDefaultHeight = 480;

    // Window's width and height in pixels.
    Resolution currentResolution;

    // For each display/monitor, the supported resolutions.
    // Indexed by [displayIndex][resolutionIndex].
    std::vector<std::vector<Resolution>> supportedResolutions;
    
    void DetectSupportedResolutions()
    {
        // Clear, in case this is called multiple times.
        // Possibly monitors could be plugged in or removed, in which case calling this again is a good idea.
        supportedResolutions.clear();
        
        // For each display, generate a list of supported resolutions.
        int displayCount = SDL_GetNumVideoDisplays();
        for(int i = 0; i < displayCount; ++i)
        {
            supportedResolutions.emplace_back();
            std::vector<Resolution>& resolutions = supportedResolutions.back();

            // Iterate each display mode supported by this monitor and populate a list of resolutions.
            // NOTE: Technically, in windowed mode, ANY resolution is valid - these resolutions are really only for fullscreen.
            // NOTE: But for simplicity, we'll use the same resolution set for windowed & fullscreen modes. Other resolutions can be set in the prefs file directly.
            int displayModes = SDL_GetNumDisplayModes(i);
            for(int j = 0; j < displayModes; ++j)
            {
                SDL_DisplayMode mode;
                SDL_GetDisplayMode(i, j, &mode);

                // There are likely some supported resolutions that we can safely ignore.
                // For example, if the height is larger than the width, that isn't a great choice for this type of game!
                if(mode.h >= mode.w) { continue; }

                // GetDisplayMode will contain dupes for different supported refresh rates.
                // Only add one entry per resolution. Fortunately, the display modes are sorted so this works.
                if(resolutions.empty() || (resolutions.back().width != mode.w || resolutions.back().height != mode.h))
                {
                    Resolution res;
                    res.width = mode.w;
                    res.height = mode.h;
                    resolutions.push_back(res);
                }
            }
        }
    }
    
    void DumpVideoInfo(SDL_Window* window)
    {
        // Output drivers.
        int driverCount = SDL_GetNumVideoDrivers();
        for(int i = 0; i < driverCount; ++i)
        {
            SDL_Log("Driver %i: %s\n", i, SDL_GetVideoDriver(i));
        }
        SDL_Log("Current Driver: %s\n", SDL_GetCurrentVideoDriver());

        // Output displays.
        int displayCount = SDL_GetNumVideoDisplays();
        for(int i = 0; i < displayCount; ++i)
        {
            float ddpi = 0.0f;
            float hdpi = 0.0f;
            float vdpi = 0.0f;
            SDL_GetDisplayDPI(i, &ddpi, &hdpi, &vdpi);

            SDL_Log("Display %i: %s (%f DPI)\n", i, SDL_GetDisplayName(i), ddpi);

            SDL_DisplayMode mode;
            SDL_GetCurrentDisplayMode(i, &mode);
            SDL_Log("  Current Display Mode: (%i x %i @ %iHz)\n", mode.w, mode.h, mode.refresh_rate);

            SDL_GetDesktopDisplayMode(i, &mode);
            SDL_Log("  Desktop Display Mode: (%i x %i @ %iHz)\n", mode.w, mode.h, mode.refresh_rate);

            int displayModes = SDL_GetNumDisplayModes(i);
            SDL_Log("  Num Display Modes: %i \n", displayModes);
            for(int j = 0; j < displayModes; ++j)
            {
                SDL_GetDisplayMode(i, j, &mode);
                SDL_Log("  (%i x %i @ %iHz)\n", mode.w, mode.h, mode.refresh_rate);
            }
        }
        
        // Output display that window is currently showing on.
        SDL_Log("Current Display Index: %i\n", SDL_GetWindowDisplayIndex(window));
    }
}

void Window::Create(const char* title)
{
    // Determine whether window should be fullscreen.
    bool fullscreen = gSaveManager.GetPrefs()->GetBool(PREFS_ENGINE, PREF_FULLSCREEN, false);
    
    // Determine desired window position.
    std::string xStr = gSaveManager.GetPrefs()->GetString(PREFS_ENGINE, PREF_WINDOW_X, "center");
    int xPos = SDL_WINDOWPOS_CENTERED;
    if(!StringUtil::EqualsIgnoreCase(xStr, "center") && !StringUtil::EqualsIgnoreCase(xStr, "default"))
    {
        xPos = StringUtil::ToInt(xStr);
    }

    std::string yStr = gSaveManager.GetPrefs()->GetString(PREFS_ENGINE, PREF_WINDOW_Y, "center");
    int yPos = SDL_WINDOWPOS_CENTERED;
    if(!StringUtil::EqualsIgnoreCase(yStr, "center") && !StringUtil::EqualsIgnoreCase(yStr, "default"))
    {
        yPos = StringUtil::ToInt(yStr);
    }

    // Determine desired window size.
    currentResolution.width = gSaveManager.GetPrefs()->GetInt(PREFS_ENGINE, PREF_SCREEN_WIDTH, kDefaultWidth);
    currentResolution.height = gSaveManager.GetPrefs()->GetInt(PREFS_ENGINE, PREF_SCREEN_HEIGHT, kDefaultHeight);
    
    // Determine flags.
    Uint32 flags = 0; //TODO: is this flag even needed for OGL to work? Doesn't seem like it.
    if(fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    // These flags are just about the only thing tying window creation to specific graphics APIs.
    // They don't *seem* necessary on all platforms...but just pass them in case I suppose.
    flags |= SDL_WINDOW_OPENGL;
    //flags |= SDL_WINDOW_VULKAN; // doesn't work on Mac
    
    // Create window from preferences.
    Window::Create(title, xPos, yPos, currentResolution.width, currentResolution.height, flags);
}

void Window::Create(const char* title, int x, int y, int w, int h, Uint32 flags)
{
    // Should only create one window at a time.
    // Probably only one per run of the application...
    if(window != nullptr)
    {
        Destroy();
    }
    
    // SDL video subsystem is required to create a window.
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        return;
    }
    
    // This is good a time as any to query and store available resolutions.
    DetectSupportedResolutions();
    
    // Create the window.
    window = SDL_CreateWindow(title, x, y, w, h, flags);
    //DumpVideoInfo(window);
}

void Window::Destroy()
{
    if(window != nullptr)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
        
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

SDL_Window* Window::Get()
{
    return window;
}

void Window::SetFullscreen(bool fullscreen)
{
    // Change display mode.
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

    // Save preference.
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_FULLSCREEN, fullscreen);
}

bool Window::IsFullscreen()
{
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void Window::ToggleFullscreen()
{
    // Get current setting and toggle it.
    bool isFullscreen = IsFullscreen();
    SetFullscreen(!isFullscreen);
}

const std::vector<Window::Resolution>& Window::GetResolutions()
{
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    return supportedResolutions[displayIndex];
}

const Window::Resolution& Window::GetResolution()
{
    return currentResolution;
}

void Window::SetResolution(const Resolution& resolution)
{
    int width = resolution.width;
    int height = resolution.height;
    
    // The way we set the window size depends on whether we're fullscreen or not.
    bool isFullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
    if(!isFullscreen)
    {
        // When not fullscreen, just set the window size.
        // And there's no technical limitation on the window size really.
        SDL_SetWindowSize(window, width, height);
    }
    else
    {
        // In fullscreen, we've got to use a resolution supported by the monitor.
        // Grab the display index the game is currently presenting on.
        int displayIndex = SDL_GetWindowDisplayIndex(window);

        // Create a desired mode with given width/height. Assume 60Hz and whatever pixel format is fine.
        SDL_DisplayMode desiredMode;
        desiredMode.w = width;
        desiredMode.h = height;
        desiredMode.refresh_rate = 60;
        desiredMode.format = SDL_PIXELFORMAT_UNKNOWN;

        // Find the closest supported mode to our desired mode.
        SDL_DisplayMode supportedMode;
        SDL_GetClosestDisplayMode(displayIndex, &desiredMode, &supportedMode);

        // Set the display mode.
        SDL_SetWindowDisplayMode(window, &supportedMode);

        // Update passed width/height to match supported mode (so we save a valid pref).
        width = supportedMode.w;
        height = supportedMode.h;
    }
    
    // Save preference.
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_SCREEN_WIDTH, width);
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_SCREEN_HEIGHT, height);

    // Save new resolution.
    currentResolution.width = width;
    currentResolution.height = height;
}

void Window::SetPosition(int x, int y)
{
    SDL_SetWindowPosition(window, x, y);
}

void Window::OnPositionChanged()
{
    // Save new display index, in case window moved to new display.
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_MONITOR, displayIndex);
    
    // Save new x/y position of window.
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_WINDOW_X, x);
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_WINDOW_Y, y);
}
