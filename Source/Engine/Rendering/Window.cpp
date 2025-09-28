#include "Window.h"

#include <map>
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

    int ResolutionHash(const Resolution& res)
    {
        // Not a great hash...but it does the trick.
        return static_cast<int>(res.width) * 1000 + static_cast<int>(res.height);
    }

    void DetectSupportedResolutions(bool fullscreen)
    {
        // Clear, in case this is called multiple times.
        // Possibly monitors could be plugged in or removed, in which case calling this again is a good idea.
        supportedResolutions.clear();

        // For each display, generate a list of supported resolutions.
        int displayCount = SDL_GetNumVideoDisplays();
        for(int i = 0; i < displayCount; ++i)
        {
            // As we iterate display modes, we'll keep a map of supported resolutions. Using a map enables two things:
            // 1) Resolutions are ordered.
            // 2) Avoid duplicate resolutions.
            std::map<int, Resolution> resolutions;

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

                Resolution res;
                res.width = static_cast<uint32_t>(mode.w);
                res.height = static_cast<uint32_t>(mode.h);
                resolutions[ResolutionHash(res)] = res;
            }

            // If not fullscreen, ensure a few common/expected resolutions are present.
            // Particularly on Linux, the display mode enumeration may not be exhaustive.
            if(!fullscreen)
            {
                // Query the desktop display mode, so we know the max resolution we should offer.
                // This may not be 100% correct, but it seems reasonable you wouldn't want a window larger than your desktop?
                SDL_DisplayMode desktopDisplayMode;
                SDL_GetDesktopDisplayMode(i, &desktopDisplayMode);

                // Authored list of common resolutions.
                static Resolution sCommonResolutions[] = {
                    { 640, 480 },
                    { 800, 600 },
                    { 1024, 768 },
                    { 1366, 768 },
                    { 1440, 900 },
                    { 1600, 900 },
                    { 1920, 1080 },
                    { 1920, 1200 },
                    { 2560, 1440 },
                    { 2560, 1600 },
                    { 3440, 1440 },
                    { 3840, 2160 }
                };

                // Add each common resolution, as long as it fits within desktop resolution.
                for(auto& res : sCommonResolutions)
                {
                    if(res.width <= desktopDisplayMode.w && res.height <= desktopDisplayMode.h)
                    {
                        resolutions[ResolutionHash(res)] = res;
                    }
                }
            }

            // Convert resolutions map into a list.
            supportedResolutions.emplace_back();
            for(auto& entry : resolutions)
            {
                supportedResolutions.back().push_back(entry.second);
            }
        }
    }

    void DumpVideoInfo(SDL_Window* dumpWindow)
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
        SDL_Log("Current Display Index: %i\n", SDL_GetWindowDisplayIndex(dumpWindow));
    }

    void PositionWindowSanely()
    {
        // If fullscreen, the window is not visible/moveable, so we don't need to do anything.
        if(IsFullscreen()) { return; }

        // Get the window size and the desktop screen resolution size.
        int windowWidth = GetWidth();
        int windowHeight = GetHeight();
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode);

        // One scenario to account for: the window size exactly matches the desktop size. This is like a "fullscreen window".
        // In this case, just position the window at (0, 0) so it takes up the whole screen.
        if(windowWidth == mode.w && windowHeight == mode.h)
        {
            SetPosition(0, 0);
            return;
        }

        // Another scenario is that the game window is larger than the desktop resolution.
        // This is kind of a weird state that isn't typically allowed. (TODO: We could force the window to the desktop resolution?)
        if(windowWidth > mode.w || windowHeight > mode.h)
        {
            return;
        }

        // Otherwise, the window is smaller than the desktop, so the player can move it wherever they'd like.

        // In windowed mode, when fiddling with resolutions and fullscreen toggles, you can sometimes get in an unfortunate situation:
        // The window title bar is off-screen, and you can't grab it anymore!
        // Sure, you could maybe use a keyboard shortcut to reposition the window, but it's a tricky spot to be in.
        // To alleviate the problem, we should try to keep the title bar on-screen.

        // See if the y-position is off-screen at the top. If so, nudge the window down.
        // "32" is kind of arbitrary, but is the height of the Windows title bar.
        //TODO: May be able to use SDL_GetWindowBordersSize to get the title bar size from the OS?
        const int kTitleBarHeight = 32;
        int x;
        int y;
        SDL_GetWindowPosition(window, &x, &y);
        if(y < kTitleBarHeight)
        {
            SetPosition(x, kTitleBarHeight);
        }
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
    Uint32 flags = 0;
    if(fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    // These flags are just about the only thing tying window creation to specific graphics APIs.
    // They don't *seem* necessary on all platforms...but just pass them in case I suppose.
    flags |= SDL_WINDOW_OPENGL; //TODO: is this flag even needed for OGL to work? Doesn't seem like it.
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
    DetectSupportedResolutions((flags & SDL_WINDOW_FULLSCREEN) != 0);

    // Create the window.
    window = SDL_CreateWindow(title, x, y, w, h, flags);
    if(window == nullptr)
    {
        printf("Failed to create SDL window! Error: %s\n", SDL_GetError());
    }
    //DumpVideoInfo(window);

    // Make sure window is in a sane position.
    PositionWindowSanely();
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

    // Update supported resolutions list.
    DetectSupportedResolutions(fullscreen);

    // Make sure window is in a sane position.
    PositionWindowSanely();
}

bool Window::IsFullscreen()
{
    // Returns "true" for both "fullscreen exclusive" AND "fullscreen desktop".
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void Window::ToggleFullscreen()
{
    SetFullscreen(!IsFullscreen());
}

Window::Mode Window::GetFullscreenMode()
{
    Uint32 flags = SDL_GetWindowFlags(window);
    bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    bool isFullscreenDesktop = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;

    // Exclusive fullscreen reports as fullscreen, but not fullscreen desktop.
    if(isFullscreen && !isFullscreenDesktop)
    {
        return Mode::FullscreenExclusive;
    }
    if(isFullscreen)
    {
        return Mode::FullscreenDesktop;
    }
    return Mode::Windowed;
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
    int width = static_cast<int>(resolution.width);
    int height = static_cast<int>(resolution.height);

    // The way we set the window size depends on whether we're fullscreen or not.
    if(IsFullscreen())
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

    // Regardless of fullscreen or not, it *seems* that you must set the window size as well for the correct result.
    // If you don't do this in fullscreen mode, changed resolutions are cropped or don't take up the full screen as expected.
    SDL_SetWindowSize(window, width, height);

    // Save preference.
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_SCREEN_WIDTH, width);
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_SCREEN_HEIGHT, height);

    // Save new resolution.
    currentResolution.width = width;
    currentResolution.height = height;

    // Make sure window is in a sane position.
    PositionWindowSanely();
}

Vector2 Window::GetPosition()
{
    int x;
    int y;
    SDL_GetWindowPosition(window, &x, &y);
    return Vector2(x, y);
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

void Window::SetTitle(const char* title)
{
    SDL_SetWindowTitle(window, title);
}

const char* Window::GetTitle()
{
    return SDL_GetWindowTitle(window);
}