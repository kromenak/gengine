//
// Clark Kromenaker
//
// The "main" engine class. This is created by main() and handles
// init, running the game loop, shutdown, and some coordination between systems.
//
#pragma once
#include <cstdint>

#include "Timers.h"

class PersistState;

class GEngine
{
public:
    static GEngine* Instance() { return sInstance; }

    GEngine();

    bool Initialize();
    void Shutdown();
    void Run();

    void Quit();

    uint32_t GetFrameNumber() const { return mFrameNumber; }

    void SetTimeMultiplier(float multiplier) { mTimeMultiplier = multiplier; }
    float GetTimeMultiplier() const { return mTimeMultiplier; }

    void UpdateGameWorld(float deltaTime);

    // GK-specific stuff here
    void StartGame() const;
    bool IsDemoMode() const { return mDemoMode; }
    void SetAllowInteractDuringActions(bool allow) { mAllowInteractDuringActions = allow; }

    void OnPersist(PersistState& ps);

private:
    // Only one instance of GEngine can exist.
    static GEngine* sInstance;

    // Is the game running? While true, we loop. When false, the game exits.
    // False by default, but set to true after initialization.
    bool mRunning = false;

    // Tracks what frame the game is on. First full frame execution is frame 0.
    // Assuming 60FPS, it would take ~800 days for this value to wrap. Not too concerning.
    uint32_t mFrameNumber = 0;

    // Used to track how much time has passed each frame.
    DeltaTimer mDeltaTimer;

    // A multiplier to affect how fast the game updates compared to realtime.
    // Less than 1 makes the game run slower than realtime, more than 1 makes it run faster than realtime.
    float mTimeMultiplier = 1.0f;

    // If true, the game runs as the "demo" version of the game.
    bool mDemoMode = false;

    // If true, interacting is allowed during actions.
    // Typically this isn't allowed, but it's overridden for the settings menu.
    bool mAllowInteractDuringActions = false;

    // If true, the application is focused (not backgrounded).
    bool mApplicationFocused = true;

    // If true, the application continues to update, even when it doesn't have focus.
    bool mAlwaysActive = false;

    void ShowOpeningMovies();
    void ShowTitleScreen();

    void ProcessInput();
    void Update();
    void GenerateOutputs();
};
