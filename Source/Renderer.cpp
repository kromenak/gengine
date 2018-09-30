//
//  Renderer.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Actor.h"
#include "BSP.h"
#include "CameraComponent.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Shader.h"
#include "Skybox.h"
#include "UIWidget.h"

float axis_vertices[] = {
    0.0f, 0.0f, 0.0f,
    5.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 5.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 5.0f
};

float axis_colors[] = {
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f
};

Mesh* axes = nullptr;

float quad_vertices[] = {
	-0.5f,  0.5f, 0.0f, // upper-left
	 0.5f,  0.5f, 0.0f, // upper-right
	 0.5f, -0.5f, 0.0f, // lower-right
	-0.5f, -0.5f, 0.0f, // lower-left
};

float quad_uvs[] = {
	0.0f, 0.0f,		// upper-left
	1.0f, 0.0f,		// upper-right
	1.0f, 1.0f,		// lower-right
	0.0f, 1.0f		// lower-left
};

unsigned short quad_indices[] = {
	0, 1, 2, 	// upper-right triangle
	2, 3, 0		// lower-left triangle
};

Mesh* quad = nullptr;

bool Renderer::Initialize()
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
    
    // Load default shader.
	mDefaultShader = LoadShader("3D-Diffuse-Tex");
	Material::sDefaultShader = mDefaultShader;
	if(mDefaultShader == nullptr) { return false; }
	
    // Compile skybox shader.
    mSkyboxShader = LoadShader("3D-Skybox");
    if(mSkyboxShader == nullptr) { return false; }
    
    // Create axes mesh, which is helpful for debugging.
    axes = new Mesh(6, 7 * sizeof(float), MeshUsage::Static);
    axes->SetRenderMode(RenderMode::Lines);
    axes->SetPositions(axis_vertices);
    axes->SetColors(axis_colors);
	
	// Create quad mesh, which is used for UI and 2D rendering.
	quad = new Mesh(4, 5 * sizeof(float), MeshUsage::Static);
	quad->SetRenderMode(RenderMode::Triangles);
	quad->SetPositions(quad_vertices);
	quad->SetUV1(quad_uvs);
	quad->SetIndexes(quad_indices, 6);
    
    // Init succeeded!
    return true;
}

void Renderer::Shutdown()
{
    delete mDefaultShader;
    delete mSkyboxShader;
    
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Render()
{
    // Don't render if there's no camera.
    if(mCameraComponent == nullptr) { return; }
    
    // Draws a little axes indicator at world origin.
	mDefaultShader->SetUniformMatrix4("uWorldTransform", Matrix4::Identity);
    glBindTexture(GL_TEXTURE_2D, 0);
    axes->Render();
    
    // We'll need the projection matrix a few times below.
    // We'll also calculate the view/proj combined matrix one or two times.
    Matrix4 projectionMatrix = mCameraComponent->GetProjectionMatrix();
    Matrix4 viewProjMatrix;
    
    // Draw the skybox first, if we have one.
    // For skybox, we don't want to write into the depth mask, or else you can ONLY see skybox.
    // This is because the skybox is actually just a small cube around the camera.
    glDepthMask(GL_FALSE); // stops writing to depth buffer
    if(mSkybox != nullptr)
    {
        // To get the "infinite distance" skybox effect, we need to use a look-at
        // matrix that doesn't take the camera's position into account.
        viewProjMatrix = projectionMatrix * mCameraComponent->GetLookAtMatrixNoTranslate();
        
        mSkyboxShader->Activate();
        mSkyboxShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
        
        mSkybox->Render();
    }
    glDepthMask(GL_TRUE); // start writing to depth buffer
    
    // For the rest of rendering, the normal view/proj matrix is fine.
    viewProjMatrix = projectionMatrix * mCameraComponent->GetLookAtMatrix();
    
    // Enable depth test and disable blend to draw opaque 3D geometry.
    glEnable(GL_DEPTH_TEST); // do depth comparisons and update the depth buffer
    glDisable(GL_BLEND); // do not perform alpha blending (opaque rendering)
    
    // Activate, for now, our one and only shader.
    mDefaultShader->Activate();
    
    // Set the combined view/projection matrix based on the assigned camera.
    mDefaultShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
    
    // Render an axis at the world origin for debugging.
    glBindTexture(GL_TEXTURE_2D, 0);
    axes->Render();
    
    // Render all mesh components. (should do before or after BSP?)
    std::vector<RenderPacket> allRenderPackets;
    for(auto& meshRenderer : mMeshRenderers)
    {
        std::vector<RenderPacket> packets = meshRenderer->GetRenderPackets();
        allRenderPackets.insert(allRenderPackets.end(), packets.begin(), packets.end());
        //meshRenderer->Render();
    }
    
    // Render the packets.
    for(auto& packet : allRenderPackets)
    {
        packet.Render();
    }
    
    // Reset world matrix to identity for BSP rendering (or for next render cycle).
	mDefaultShader->SetUniformMatrix4("uWorldTransform", Matrix4::Identity);
    
    // From here, we need alpha blending.
    // Since we render opaque and alpha BSP in one go, it needs to be on for BSP.
    // And then we certainly need alpha blending for UI rendering.
    glEnable(GL_BLEND); // do alpha blending (transparent rendering)
    glDepthMask(GL_FALSE); // don't write to the depth buffer
    
    // Render the BSP.
    if(mBSP != nullptr)
    {
        mBSP->Render(mCameraComponent->GetOwner()->GetPosition());
    }
    
    // Enable alpha-blended rendering and render UI elements.
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	mDefaultShader->SetUniformMatrix4("uViewProj", Matrix4::MakeSimpleScreenOrtho(GetWindowWidth(), GetWindowHeight()));
	glDisable(GL_DEPTH_TEST);
	
    // Render UI elements.
    for(auto& widget : mWidgets)
    {
        widget->Render();
    }
	
    // Reset for next render loop.
	//mDefaultShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
    glDisable(GL_BLEND); // do not perform alpha blending (opaque rendering)
    glDepthMask(GL_TRUE); // start writing to depth buffer
	glEnable(GL_DEPTH_TEST);
}

void Renderer::Present()
{
    SDL_GL_SwapWindow(mWindow);
}

void Renderer::AddMeshRenderer(MeshRenderer* mr)
{
    mMeshRenderers.push_back(mr);
}

void Renderer::RemoveMeshRenderer(MeshRenderer* mr)
{
    auto it = std::find(mMeshRenderers.begin(), mMeshRenderers.end(), mr);
    if(it != mMeshRenderers.end())
    {
        mMeshRenderers.erase(it);
    }
}

void Renderer::AddUIWidget(UIWidget* widget)
{
    mWidgets.push_back(widget);
}

void Renderer::RemoveUIWidget(UIWidget* widget)
{
    auto it = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if(it != mWidgets.end())
    {
        mWidgets.erase(it);
    }
}

Shader* Renderer::LoadShader(std::string name)
{
	// Load shader asset from AssetManager.
	// TODO: Support for loading from text directly?
	Shader* shader = Services::GetAssets()->LoadShader(name);
	
	// If shader couldn't be found, or failed to load for some reason, return null.
	if(shader == nullptr || !shader->IsGood())
	{
		return nullptr;
	}
	
	// Add to shader vector.
	mShaders.push_back(shader);
	
	// Success!
	return shader;
}
