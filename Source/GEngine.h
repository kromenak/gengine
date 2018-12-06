// Clark Kromenaker
//
// The "main" engine class. This is created by main() and handles
// init, running the game loop, shutdown, and some coordination between systems.
//
#pragma once
#include <vector>

#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "SheepManager.h"

class Actor;
class Scene;
class Cursor;

class GEngine
{
public:
    static void AddActor(Actor* actor);
    static void RemoveActor(Actor* actor);
    static GEngine* inst;
    
    GEngine();
    
    bool Initialize();
    void Shutdown();
    void Run();
    
    void Quit();
    
    void LoadScene(std::string name);
    Scene* GetScene() { return mScene; }
    
    std::string GetCurrentTimeCode();
    
private:
    // A list of all actors that currently exist in the game.
    static std::vector<Actor*> mActors;
    
    // Is the game running? While true, we loop. When false, the game exits.
    bool mRunning;
    
    // Subsystems.
    Renderer mRenderer;
    AudioManager mAudioManager;
    AssetManager mAssetManager;
    InputManager mInputManager;
    SheepManager mSheepManager;
    
    // The currently active scene. There can be only one at a time.
    Scene* mScene = nullptr;
    
    Cursor* mCursor = nullptr;
    
    // Day and time that the game is currently in.
    // GK3 takes place over three days and multiple time blocks.
    int mDay = 1;
    int mHour = 10; // hour in a 24-hour clock
    
    void ProcessInput();
    void Update();
    void GenerateOutputs();
};
