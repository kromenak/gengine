//
//  SDLRenderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once

#include "SDL/SDL.h"

class SDLRenderer
{
public:
    bool Initialize();
    void Shutdown();
    
    void Present();
    
private:
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
};
