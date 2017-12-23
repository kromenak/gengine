//
//  SDLRenderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once

#include "SDL/SDL.h"
#include <GL/glew.h>
#include "GLVertexArray.h"
#include "GLShader.h"

class CameraComponent;
class Model;

class SDLRenderer
{
public:
    bool Initialize();
    void Shutdown();
    
    void Clear();
    void Render();
    void Present();
    
    void SetModel(Model* model);
    
    void SetCamera(CameraComponent* camera) { mCameraComponent = camera; }
    
private:
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context for rendering in OpenGL.
    SDL_GLContext mContext;
    
    GLShader* mShader = nullptr;
    
    Model* mModel = nullptr;
    GLVertexArray* mVertArray = nullptr;
    
    CameraComponent* mCameraComponent = nullptr;
};
