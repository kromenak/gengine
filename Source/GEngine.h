//
// Clark Kromenaker
//
// The "main" engine class. This is created by main() and handles
// init, running the game loop, shutdown, and some coordination between systems.
//
#pragma once
#include <cstdint>

class GEngine
{
public:
    static GEngine* Instance() { return sInstance; }
    
    GEngine();
    
    bool Initialize();
    void Shutdown();
    void Run();
    
    void Quit();

    void ForceUpdate();

    uint32_t GetFrameNumber() const { return mFrameNumber; }

    void SetTimeMultiplier(float multiplier) { mTimeMultiplier = multiplier; }
    float GetTimeMultiplier() const { return mTimeMultiplier; }

    // GK-specific stuff here
    void StartGame() const;
    bool IsDemoMode() const { return mDemoMode; }
    
private:
    // Only one instance of GEngine can exist.
    static GEngine* sInstance;
    
    // Is the game running? While true, we loop. When false, the game exits.
	// False by default, but set to true after initialization.
	bool mRunning = false;

    // Tracks what frame the game is on. First full frame execution is frame 0.
    // Assuming 60FPS, it would take ~800 days for this value to wrap. Not too concerning.
    uint32_t mFrameNumber = 0;

    // A multiplier to affect how fast the game updates compared to realtime.
    // Less than 1 makes the game run slower than realtime, more than 1 makes it run faster than realtime.
    float mTimeMultiplier = 1.0f;

    // If true, the game runs as the "demo" version of the game.
    bool mDemoMode = false;

    void ShowOpeningMovies();
    void ShowTitleScreen();

    void ProcessInput();
    void Update();
    void Update(float deltaTime);
    void GenerateOutputs();
};
