//
//  Renderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "Material.h"
#include "Matrix4.h"
#include "Rect.h"
#include "Vector2.h"

class BSP;
class Camera;
class MeshRenderer;
class Model;
class Shader;
class Skybox;

class Renderer
{
public:
    bool Initialize();
    void Shutdown();
    
    void Render();
    
    void SetCamera(Camera* camera) { mCamera = camera; }
    Camera* GetCamera() { return mCamera; }
	
    void AddMeshRenderer(MeshRenderer* mc);
    void RemoveMeshRenderer(MeshRenderer* mc);
    
    void SetBSP(BSP* bsp) { mBSP = bsp; }
    
	void SetSkybox(Skybox* skybox);

    void ToggleFullscreen();

    int GetWindowWidth() { return mScreenWidth; }
	int GetWindowHeight() { return mScreenHeight; }
	Vector2 GetWindowSize() { return Vector2(static_cast<float>(mScreenWidth), static_cast<float>(mScreenHeight)); }
    Rect GetScreenRect() { return Rect(0, 0, mScreenWidth, mScreenHeight); }

    void SetUseMipmaps(bool useMipmaps);
    void SetUseTrilinearFiltering(bool useTrilinearFiltering);
    bool UseMipmaps() const { return mUseMipmaps; }
    bool UseTrilinearFiltering() const { return mUseTrilinearFiltering; }
    
private:
    // Screen's width and height, in pixels.
    int mScreenWidth = 800;
    int mScreenHeight = 600;
    
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context handle for rendering in OpenGL.
    SDL_GLContext mContext = nullptr;
    
    // Our camera in the scene - we currently only support one.
    Camera* mCamera = nullptr;
    
    // List of mesh components to render.
    std::vector<MeshRenderer*> mMeshRenderers;
	
    // A BSP to render.
    BSP* mBSP = nullptr;
    
    // A skybox to render.
	Material mSkyboxMaterial;
    Skybox* mSkybox = nullptr;

    // Global texture settings.
    bool mUseMipmaps = true;
    bool mUseTrilinearFiltering = true;
};
