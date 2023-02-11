//
//  Renderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once
#include <string>
#include <vector>

#include <SDL.h>
#include <GL/glew.h>

#include "Matrix4.h"
#include "Rect.h"
#include "Vector2.h"
#include "Window.h"

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

    void Clear();
    void Render();
    void Present();
    
    void SetCamera(Camera* camera) { mCamera = camera; }
    Camera* GetCamera() { return mCamera; }
	
    void AddMeshRenderer(MeshRenderer* mc);
    void RemoveMeshRenderer(MeshRenderer* mc);
    
    void SetBSP(BSP* bsp) { mBSP = bsp; }
    
	void SetSkybox(Skybox* skybox);
    
    void SetUseMipmaps(bool useMipmaps);
    void SetUseTrilinearFiltering(bool useTrilinearFiltering);
    bool UseMipmaps() const { return mUseMipmaps; }
    bool UseTrilinearFiltering() const { return mUseTrilinearFiltering; }

    void ChangeResolution(const Window::Resolution& resolution);

    SDL_GLContext GetGLContext() { return mContext; }

private:
    // Context handle for rendering in OpenGL.
    SDL_GLContext mContext = nullptr;
    
    // Our camera in the scene - we currently only support one.
    Camera* mCamera = nullptr;
    
    // List of mesh components to render.
    std::vector<MeshRenderer*> mMeshRenderers;
	
    // A BSP to render.
    BSP* mBSP = nullptr;
    
    // A skybox to render.
    Skybox* mSkybox = nullptr;

    // Global texture settings.
    bool mUseMipmaps = true;
    bool mUseTrilinearFiltering = true;
};
