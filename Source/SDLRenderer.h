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
#include "Matrix4.h"
#include <vector>

class CameraComponent;
class MeshComponent;
class Model;

class SDLRenderer
{
public:
    bool Initialize();
    void Shutdown();
    
    void Clear();
    void Render();
    void Present();
    
    void SetCamera(CameraComponent* camera) { mCameraComponent = camera; }
    
    void SetWorldTransformMatrix(Matrix4& worldTransform);
    
    void AddMeshComponent(MeshComponent* mc);
    void RemoveMeshComponent(MeshComponent* mc);
    
    GLShader* GetShader() { return mShader; }
    
private:
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context for rendering in OpenGL.
    SDL_GLContext mContext;
    
    // Default shader.
    GLShader* mShader = nullptr;
    
    // Our camera in the scene - we currently only support one.
    CameraComponent* mCameraComponent = nullptr;
    
    // List of mesh components to render.
    std::vector<MeshComponent*> mMeshComponents;
};
