//
//  SDLRenderer.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//

#include "SDLRenderer.h"

bool SDLRenderer::Initialize()
{
    // Init video subsystem.
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }
    
    // Create a window.
    mWindow = SDL_CreateWindow("Title", 100, 100, 1024, 768, 0);
    if(!mWindow)
    {
        return false;
    }
    
    // Attach rendering context to the window.
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!mRenderer)
    {
        return false;
    }
    
    // Init succeeded!
    return true;
}

void SDLRenderer::Shutdown()
{
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
}

void SDLRenderer::Present()
{
    //SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    //SDL_RenderClear(mRenderer);
    SDL_RenderPresent(mRenderer);
}
