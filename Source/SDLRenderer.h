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
    
    void Clear();
    void Render();
    void Present();
    
private:
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context for rendering in OpenGL.
    SDL_GLContext mContext;
    
    // SDL renderer context, for 2D rendering.
    //SDL_Renderer* mRenderer;
};
