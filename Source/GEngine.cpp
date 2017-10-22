//
//  GEngine.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//

#include "GEngine.h"
#include "SDL/SDL.h"

#include "Sheep/SheepDriver.h"
#include "Barn/BarnFile.h"

#include "Actor.h"

GEngine::GEngine() : mRunning(false)
{
    
}

bool GEngine::Initialize()
{
    if(!mRenderer.Initialize())
    {
        return false;
    }
    
    if(!mAudio.Initialize())
    {
        return false;
    }
    
    //Sheep::Driver driver;
    //driver.Parse("/Users/Clark/Dropbox/GK3/Assets/test.shp");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/core.brn");
    //barnFile.WriteToFile("E18LCJ44QR1.YAK");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/ambient.brn");
    //barnFile.WriteToFile("R33SNEAKPEEK.WAV");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/day1.brn");
    //barnFile.WriteToFile("CASH.BMP");
    
    //BarnFile barnFile("day1.brn");
    //barnFile.WriteToFile("CASH.BMP");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/day3.brn");
    //barnFile.WriteToFile("TE4SUNBUTTON.BMP");
    
    //mAssetManager.AddSearchPath("Assets/");
    //mAssetManager.LoadBarn("day1.brn");
    
    //Model* model = mAssetManager.LoadModel("SYRUPPACKET.MOD");
    //mRenderer.SetModel(model);
    
    //Model* model = mAssetManager.LoadModel("TAX.MOD");
    //Model* model = mAssetManager.LoadModel("R25HANGER.MOD");
    
    //BarnFile* barnFile = mAssetManager.GetBarn("day1.brn");
    //barnFile->WriteToFile("WI2BARGLASS.MOD");
    
    //Audio* audio = mAssetManager.LoadAudio("HALLS3.WAV");
    //audio->WriteToFile();
    
    //mAudio.Play(*audio);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
    
    Actor* actor = new Actor();
    actor->SetScale(Vector3(3.0f, 3.0f, 3.0f));
    actor->SetRotation(Vector3(1.0f, 1.0f, 1.0f));
    actor->SetPosition(Vector3(15.0f, -30.0f, 10.0f));
    mActors.push_back(actor);
    return true;
}

void GEngine::Shutdown()
{
    // Delete all actors and clear actor list.
    for(auto& actor : mActors)
    {
        delete actor;
    }
    mActors.clear();
    
    mRenderer.Shutdown();
    mAudio.Shutdown();
    
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

void GEngine::ProcessInput()
{
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
    
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_ESCAPE])
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
    for(auto& actor : mActors)
    {
        actor->Update(0.0f);
    }
}

void GEngine::GenerateOutput()
{
    mRenderer.Clear();
    mRenderer.Render();
    mRenderer.Present();
}
