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
    struct Resolution
    {
        int width = 0;
        int height = 0;
    };

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

    const std::vector<Resolution>& GetResolutions();
    void SetWindowSize(int width, int height);
    void OnWindowPositionChanged();

    int GetWindowWidth() { return mCurrentResolution.width; }
	int GetWindowHeight() { return mCurrentResolution.height; }
	Vector2 GetWindowSize() { return Vector2(static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight())); }
    Rect GetWindowRect() { return Rect(0, 0, GetWindowWidth(), GetWindowHeight()); }

    void SetUseMipmaps(bool useMipmaps);
    void SetUseTrilinearFiltering(bool useTrilinearFiltering);
    bool UseMipmaps() const { return mUseMipmaps; }
    bool UseTrilinearFiltering() const { return mUseTrilinearFiltering; }
    
private:
    // Default width & height on first run, or if no other preference is specified.
    const int kDefaultScreenWidth = 640;
    const int kDefaultScreenHeight = 480;

    // Screen's width and height, in pixels.
    Resolution mCurrentResolution;

    // Possible resolutions.
    std::vector<std::vector<Resolution>> mResolutions;
    
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

    void DetectAvailableResolutions();
};
