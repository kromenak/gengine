//
//  Renderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "Matrix4.h"
#include <vector>

class Shader;
class CameraComponent;
class MeshRenderer;
class Model;
class BSP;
class Skybox;

class Renderer
{
public:
    bool Initialize();
    void Shutdown();
    
    void Clear();
    void Render();
    void Present();
    
    void SetCamera(CameraComponent* camera) { mCameraComponent = camera; }
    CameraComponent* GetCamera() { return mCameraComponent; }
    
    void SetWorldTransformMatrix(Matrix4& worldTransform);
    
    void AddMeshRenderer(MeshRenderer* mc);
    void RemoveMeshRenderer(MeshRenderer* mc);
    
    void SetBSP(BSP* bsp) { mBSP = bsp; }
    
    void SetSkybox(Skybox* skybox) { mSkybox = skybox; }
    
    Shader* GetShader() { return mDefaultShader; }
    
    int GetWidth() { return mScreenWidth; }
    int GetHeight() { return mScreenHeight; }
    
private:
    // Screen's width and height, in pixels.
    int mScreenWidth = 1024;
    int mScreenHeight = 768;
    
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context for rendering in OpenGL.
    SDL_GLContext mContext;
    
    // Default shader.
    Shader* mDefaultShader = nullptr;
    
    // Our camera in the scene - we currently only support one.
    CameraComponent* mCameraComponent = nullptr;
    
    // List of mesh components to render.
    std::vector<MeshRenderer*> mMeshRenderers;
    
    // A BSP to render.
    BSP* mBSP = nullptr;
    
    // A skybox to render.
    Skybox* mSkybox = nullptr;
    
    // Skybox shader.
    Shader* mSkyboxShader = nullptr;
};
