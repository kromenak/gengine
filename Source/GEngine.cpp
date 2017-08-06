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
    
    //Sheep::Driver driver;
    //driver.Parse("/Users/Clark/Dropbox/GK3/Assets/test.shp");
    
    BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/core.brn");
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/ambient.brn");
    
    return true;
}

void GEngine::Shutdown()
{
    mRenderer.Shutdown();
    
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
