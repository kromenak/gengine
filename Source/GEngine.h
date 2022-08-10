//
// Clark Kromenaker
//
// The "main" engine class. This is created by main() and handles
// init, running the game loop, shutdown, and some coordination between systems.
//
#pragma once
#include <functional>
#include <vector>

#include "ActionManager.h"
#include "Atomics.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "Console.h"
#include "CursorManager.h"
#include "InputManager.h"
#include "LayerManager.h"
#include "Renderer.h"
#include "SheepManager.h"
#include "ReportManager.h"
#include "VideoPlayer.h"

class Actor;
class Scene;

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

    void AddActor(Actor* actor) { mActors.push_back(actor); }
    const std::vector<Actor*>& GetActors() const { return mActors; }
    
    void LoadScene(const std::string& name, std::function<void()> callback = nullptr);
    
    Scene* GetScene() { return mScene; }

    uint32 GetFrameNumber() const { return mFrameNumber; }

    // GK-specific stuff here
    void StartGame();
    
private:
    // Only one instance of GEngine can exist.
    static GEngine* sInstance;
    
    // Is the game running? While true, we loop. When false, the game exits.
	// False by default, but set to true after initialization.
	bool mRunning = false;

    // Tracks what frame the game is on. First full frame execution is frame 0.
    // Assuming 60FPS, it would take ~800 days for this value to wrap. Not too concerning.
    uint32 mFrameNumber = 0;
    
    // Subsystems.
    Renderer mRenderer;
    AudioManager mAudioManager;
    AssetManager mAssetManager;
    InputManager mInputManager;
    SheepManager mSheepManager;
	ReportManager mReportManager;
	ActionManager mActionManager;
	Console mConsole;
    VideoPlayer mVideoPlayer;
    LayerManager mLayerManager;
    CursorManager mCursorManager;
    
    // A list of all actors that currently exist in the game.
    std::vector<Actor*> mActors;
    
    // The currently active scene. There can be only one at a time (sure about that?).
    Scene* mScene = nullptr;
	
	// A scene that's been requested to load. If empty, no pending scene change.
	// Scene loads happen at the end of a frame, to avoid a scene change mid-frame.
	std::string mSceneToLoad;

    // Callback to execute when scene load completes.
    std::function<void()> mSceneLoadedCallback = nullptr;
	
    void ProcessInput();
    void Update();
    void Update(float deltaTime);
    void GenerateOutputs();
	
	void LoadSceneInternal();
    void UnloadScene();
	
	void DeleteDestroyedActors();
};
