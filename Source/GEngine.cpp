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
    
    
    mAssetManager.AddSearchPath("Assets/");
    mAssetManager.LoadBarn("ambient.brn");
    
    Audio* audio = mAssetManager.LoadAudio("HALLS3.WAV");
    audio->WriteToFile();
    
    //mAudio.Play(*audio);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
    return true;
}

void GEngine::Shutdown()
{
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
    
}

void GEngine::GenerateOutput()
{
    mRenderer.Present();
}
