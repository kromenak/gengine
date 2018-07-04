//
//  SDLRenderer.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "SDLRenderer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Matrix4.h"
#include "Model.h"
#include "CameraComponent.h"
#include "MeshComponent.h"
#include "Skybox.h"

GLfloat axis_vertices[] = {
    0.0f, 0.0f, 0.0f,
    5.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 5.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 5.0f
};

GLfloat axis_colors[] = {
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f
};

GLVertexArray* axes = nullptr;

bool SDLRenderer::Initialize()
{
    // Init video subsystem.
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }
    
    // Tell SDL we want to use OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    // Request some GL parameters, just in case
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    // Create a window.
    mWindow = SDL_CreateWindow("GK3", 100, 100, mScreenWidth, mScreenHeight, SDL_WINDOW_OPENGL);
    if(!mWindow) { return false; }
    
    // Create OpenGL context.
    mContext = SDL_GL_CreateContext(mWindow);
    
    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
    
    // Clear any GLEW error.
    glGetError();
    
    // Our clear color will be BLACK!
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // For use with alpha blending during render loop.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Compile default shader.
    mShader = new GLShader("Assets/3D-Diffuse-Tex.vert", "Assets/3D-Diffuse-Tex.frag");
    if(!mShader->IsGood()) { return false; }
    
    // Compile skybox shader.
    mSkyboxShader = new GLShader("Assets/3D-Skybox.vert", "Assets/3D-Skybox.frag");
    if(!mSkyboxShader->IsGood()) { return false; }
    
    // Create axes mesh, which is helpful for debugging.
    axes = new GLVertexArray(axis_vertices, 18);
    axes->SetColors(axis_colors, 24);
    
    // Init succeeded!
    return true;
}

void SDLRenderer::Shutdown()
{
    delete mShader;
    delete mSkyboxShader;
    
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLRenderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SDLRenderer::Render()
{
    if(mCameraComponent == nullptr) { return; }
    
    // We'll need the projection matrix a few times below.
    // We'll also calculate the view/proj combined matrix one or two times.
    Matrix4 projectionMatrix = mCameraComponent->GetProjectionMatrix();
    Matrix4 viewProjMatrix;
    
    // Draw the skybox first, if we have one.
    // For skybox, we don't want to write into the depth mask, or else you can ONLY see skybox.
    // This is because the skybox is actually just a small cube around the camera.
    glDepthMask(GL_FALSE);
    if(mSkybox != nullptr)
    {
        // To get the "infinite distance" skybox effect, we need to use a look-at
        // matrix that doesn't take the camera's position into account.
        viewProjMatrix = projectionMatrix * mCameraComponent->GetLookAtMatrixNoTranslate();
        
        mSkyboxShader->Activate();
        mSkyboxShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
        
        mSkybox->Render();
    }
    glDepthMask(GL_TRUE);
    
    // For the rest of rendering, the normal view/proj matrix is fine.
    viewProjMatrix = mCameraComponent->GetProjectionMatrix() * mCameraComponent->GetLookAtMatrix();
    
    // Enable depth test and disable blend to draw opaque 3D geometry.
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    // Activate, for now, our one and only shader.
    mShader->Activate();
    
    // Set the combined view/projection matrix based on the assigned camera.
    mShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
    
    // Render an axis at the world origin for debugging.
    glBindTexture(GL_TEXTURE_2D, 0);
    axes->DrawLines();
    
    // Render all mesh components. (should do before or after BSP?)
    for(auto meshComponent : mMeshComponents)
    {
        meshComponent->Render();
    }
    
    // Reset world matrix to identity for BSP rendering (or for next render cycle).
    SetWorldTransformMatrix(Matrix4::Identity);
    
    // From here, we need alpha blending.
    // Since we render opaque and alpha BSP in one go, it needs to be on for BSP.
    // And then we certainly need alpha blending for UI rendering.
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    
    // Render the BSP.
    if(mBSP != nullptr)
    {
        mBSP->Render(mCameraComponent->GetOwner()->GetPosition());
    }
    
    //TODO: Enable alpha-blended rendering and render UI elements.
    //glEnable(GL_BLEND);
    //glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    
    // Reset for next render loop.
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void SDLRenderer::Present()
{
    SDL_GL_SwapWindow(mWindow);
}

void SDLRenderer::SetWorldTransformMatrix(Matrix4& worldTransform)
{
    mShader->SetUniformMatrix4("uWorldTransform", worldTransform);
}

void SDLRenderer::AddMeshComponent(MeshComponent *mc)
{
    mMeshComponents.push_back(mc);
}

void SDLRenderer::RemoveMeshComponent(MeshComponent *mc)
{
    auto it = std::find(mMeshComponents.begin(), mMeshComponents.end(), mc);
    if(it != mMeshComponents.end())
    {
        mMeshComponents.erase(it);
    }
}
