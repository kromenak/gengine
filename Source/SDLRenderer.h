//
//  SDLRenderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "GLVertexArray.h"
#include "GLShader.h"
#include "Matrix4.h"
#include <vector>

class CameraComponent;
class MeshComponent;
class Model;
class BSP;
class Skybox;

class SDLRenderer
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
    
    void SetVector3(std::string name, Vector3& vector);
    void SetMatrix4(std::string name, Matrix4& matrix);
    
    void AddMeshComponent(MeshComponent* mc);
    void RemoveMeshComponent(MeshComponent* mc);
    
    void SetBSP(BSP* bsp) { mBSP = bsp; }
    
    void SetSkybox(Skybox* skybox) { mSkybox = skybox; }
    
    GLShader* GetShader() { return mShader; }
    
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
    GLShader* mShader = nullptr;
    
    // Our camera in the scene - we currently only support one.
    CameraComponent* mCameraComponent = nullptr;
    
    // List of mesh components to render.
    std::vector<MeshComponent*> mMeshComponents;
    
    // A BSP to render.
    BSP* mBSP = nullptr;
    
    // A skybox to render.
    Skybox* mSkybox = nullptr;
    
    // Skybox shader.
    GLShader* mSkyboxShader = nullptr;
};
