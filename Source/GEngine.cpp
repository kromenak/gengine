//
//  GEngine.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "GEngine.h"
#include <SDL2/SDL.h>

#include "Services.h"
#include "Actor.h"

#include "Sheep/SheepCompiler.h"
#include "Barn/BarnFile.h"

#include "CameraComponent.h"
#include "MeshComponent.h"

#include "Mesh.h"
#include "Scene.h"

GEngine* GEngine::inst = nullptr;
std::vector<Actor*> GEngine::mActors;

GEngine::GEngine() : mRunning(false)
{
    inst = this;
}

bool GEngine::Initialize()
{
    // Initialize renderer.
    if(!mRenderer.Initialize())
    {
        return false;
    }
    Services::SetRenderer(&mRenderer);
    
    // Initialize audio.
    if(!mAudioManager.Initialize())
    {
        return false;
    }
    Services::SetAudio(&mAudioManager);
    
    // Initialize input.
    Services::SetInput(&mInputManager);
    
    // Initialize asset manager.
    Services::SetAssets(&mAssetManager);
    
    // For simplicity right now, let's just load all barns at once.
    mAssetManager.AddSearchPath("Assets/");
    mAssetManager.LoadBarn("ambient.brn");
    mAssetManager.LoadBarn("common.brn");
    mAssetManager.LoadBarn("core.brn");
    mAssetManager.LoadBarn("day1.brn");
    mAssetManager.LoadBarn("day2.brn");
    mAssetManager.LoadBarn("day3.brn");
    mAssetManager.LoadBarn("day23.brn");
    mAssetManager.LoadBarn("day123.brn");
    
    //mAssetManager.WriteBarnAssetToFile("DEFAULT.BMP");
    //mAssetManager.WriteOutAssetsOfType("YAK");
    
    // Initialize sheep manager.
    Services::SetSheep(&mSheepManager);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
    
    //mCursor = mAssetManager.LoadCursor("C_WAIT.CUR");
    mCursor = mAssetManager.LoadCursor("C_POINT.CUR");
    mCursor->Activate();
    
    LoadScene("B25");
    return true;
}

void GEngine::Shutdown()
{
    // Delete all actors. Since actor destructor
    // removes from this list, can't iterate and delete.s
    while(!mActors.empty())
    {
        delete mActors.back();
    }
    
    mRenderer.Shutdown();
    mAudioManager.Shutdown();
    
    //TODO: Ideally, I don't want the engine to know about SDL.
    SDL_Quit();
}

void GEngine::Run()
{
    // We are running!
    mRunning = true;
    
    // Loop until not running anymore.
    while(mRunning)
    {
        ProcessInput();
        Update();
        GenerateOutput();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

std::string GEngine::GetCurrentTimeCode()
{
    // Depending on day/hour, returns something like "110A".
    std::string ampm = (mHour <= 11) ? "A" : "P";
    int hour = mHour > 12 ? mHour - 12 : mHour;
    return std::to_string(mDay) + std::to_string(hour) + ampm;
}

void GEngine::LoadScene(std::string name)
{
    if(mScene != nullptr)
    {
        //TODO: Deinit the current stage.
        delete mScene;
        mScene = nullptr;
    }
    
    mScene = new Scene(name, GetCurrentTimeCode());
}

void GEngine::ProcessInput()
{
    // Update the input manager.
    // Retrieve input device states for us to use.
    mInputManager.Update();
    
    // We'll poll for events here. Catch the quit event.
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
        }
    }
    
    // Quit game on escape press for now.
    if(mInputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        Quit();
    }
}

void GEngine::Update()
{
    // Tracks the next "GetTicks" value that is acceptable to perform an update.
    static int nextTicks = 0;
    
    // Tracks the last ticks value each time we run this loop.
    static Uint32 lastTicks = 0;
    
    // Limit the FPS to about 60. nextTicks is always +16 at start of frame.
    // If we get here again and not 16 frames have passed, we wait.
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), nextTicks)) { }
    
    // Get current ticks and save next ticks as +16. Limits FPS to ~60.
    Uint32 currentTicks = SDL_GetTicks();
    nextTicks = currentTicks + 16;
    
    // Calculate the time delta.
    float deltaTime = ((float)(currentTicks - lastTicks)) * 0.001f;
    lastTicks = currentTicks;
    
    // Limit the time delta to, at most, 0.05 seconds.
    if(deltaTime > 0.05f) { deltaTime = 0.05f; }
    
    // Update all actors.
    for(int i = 0; i < mActors.size(); i++)
    {
        mActors[i]->Update(deltaTime);
    }
    
    // Also update audio system (before or after actors?)
    mAudioManager.Update(deltaTime);
    
    // Update active cursor.
    if(mCursor != nullptr)
    {
        mCursor->Update(deltaTime);
    }
}

void GEngine::GenerateOutput()
{
    mRenderer.Clear();
    mRenderer.Render();
    mRenderer.Present();
}

void GEngine::AddActor(Actor* actor)
{
    mActors.push_back(actor);
}

void GEngine::RemoveActor(Actor* actor)
{
    auto it = std::find(mActors.begin(), mActors.end(), actor);
    if(it != mActors.end())
    {
        mActors.erase(it);
    }
}
