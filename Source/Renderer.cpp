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
#include "Debug.h"
#include "Camera.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Shader.h"
#include "Skybox.h"
#include "Texture.h"
#include "UICanvas.h"

float line_vertices[] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f
};

float line_colors[] = {
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

Mesh* line = nullptr;

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
	
	/*
	// For debugging display count stuff...
	int displayCount = SDL_GetNumVideoDisplays();
	for(int i = 0; i < displayCount; i++)
	{
		float hdpi = 0.0f;
		float vdpi = 0.0f;
		SDL_GetDisplayDPI(i, nullptr, &hdpi, &vdpi);
		//SDL_Log("%f, %f", hdpi, vdpi);
	}
	*/
    
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
	
	// Init default textures.
	Texture::Init();
	
    // Load default shader.
	mDefaultShader = LoadShader("3D-Diffuse-Tex");
	Material::sDefaultShader = mDefaultShader;
	if(mDefaultShader == nullptr) { return false; }
	
    // Compile skybox shader.
    mSkyboxShader = LoadShader("3D-Skybox");
    if(mSkyboxShader == nullptr) { return false; }
	
	// Create line mesh, which is helpful for debugging.
	line = new Mesh(2, 7 * sizeof(float), MeshUsage::Static);
	Submesh* lineSubmesh = line->GetSubmesh(0);
	lineSubmesh->SetRenderMode(RenderMode::Lines);
	lineSubmesh->SetPositions(line_vertices);
	lineSubmesh->SetColors(line_colors);
	
    // Create axes mesh, which is helpful for debugging.
    axes = new Mesh(6, 7 * sizeof(float), MeshUsage::Static);
	Submesh* axesSubmesh = axes->GetSubmesh(0);
    axesSubmesh->SetRenderMode(RenderMode::Lines);
    axesSubmesh->SetPositions(axis_vertices);
    axesSubmesh->SetColors(axis_colors);
	
	// Create quad mesh, which is used for UI and 2D rendering.
	quad = new Mesh(4, 5 * sizeof(float), MeshUsage::Static);
	Submesh* quadSubmesh = quad->GetSubmesh(0);
	quadSubmesh->SetRenderMode(RenderMode::Triangles);
	quadSubmesh->SetPositions(quad_vertices);
	quadSubmesh->SetUV1(quad_uvs);
	quadSubmesh->SetIndexes(quad_indices, 6);
    
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

void Renderer::Render()
{
	// Enable opaque rendering (no blend, write to depth mask, test depth mask).
	// Do this BEFORE clear to avoid some glitchy graphics.
	glDisable(GL_BLEND); // do not perform alpha blending (opaque rendering)
	glDepthMask(GL_TRUE); // start writing to depth buffer
	glEnable(GL_DEPTH_TEST); // do depth comparisons and update the depth buffer
	
	// Clear color and depth buffers from last frame.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Just render nothing if there's no camera!
	if(mCamera == nullptr)
	{
		SDL_GL_SwapWindow(mWindow);
		return;
	}
	
	// We'll need the projection matrix for the camera a few times below.
	Matrix4 projectionMatrix = mCamera->GetProjectionMatrix();
	
	// Draw the skybox first, which is just a little cube around the camera.
	// Don't write to depth mask, or else you can ONLY see skybox (b/c again, little cube).
	glDepthMask(GL_FALSE); // stops writing to depth buffer
	if(mSkybox != nullptr)
	{
		// To get the "infinite distance" skybox effect, we need to use a look-at
		// matrix that doesn't take the camera's position into account.
		Matrix4 skyboxViewProjMatrix = projectionMatrix * mCamera->GetLookAtMatrixNoTranslate();
		
		mSkyboxShader->Activate();
		mSkyboxShader->SetUniformMatrix4("uViewProj", skyboxViewProjMatrix);
		
		mSkybox->Render();
	}
	glDepthMask(GL_TRUE); // start writing to depth buffer
	 
	// Calculate normal view/proj matrix, which is used for all non-skybox world rendering.
	Matrix4 viewMatrix = mCamera->GetLookAtMatrix();
	Matrix4 viewProjMatrix = projectionMatrix * viewMatrix;
	
	// Activate, for now, our one and only shader.
	mDefaultShader->Activate();
	
	// Set the combined view/projection matrix based on the assigned camera.
	//mDefaultShader->SetUniformMatrix4("uViewMatrix", viewMatrix);
	//mDefaultShader->SetUniformMatrix4("uProjectionMatrix", projectionMatrix);
	mDefaultShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
	
	// Default to world origin for now.
	mDefaultShader->SetUniformMatrix4("uWorldTransform", Matrix4::Identity);
	
	// Enable alpha test.
	mDefaultShader->SetUniformFloat("uAlphaTest", 0.1f);
	
	// Render an axis at the world origin, for debugging.
	Debug::DrawAxes(Vector3::Zero);
	
	// Render opaque BSP (front-to-back).
	if(mBSP != nullptr)
	{
		mBSP->RenderOpaque(mCamera->GetOwner()->GetPosition());
	}
	
	// Render opaque meshes (no particular order).
	// Sorting is probably not worthwhile b/c BSP likely mostly filled the z-buffer at this point.
	// And with the z-buffer, we can render opaque meshed correctly regardless of order.
	for(auto& meshRenderer : mMeshRenderers)
	{
		meshRenderer->RenderOpaque();
	}
	
	// Reset shader world transform uniform after mesh rendering.
	mDefaultShader->SetUniformMatrix4("uWorldTransform", Matrix4::Identity);
	
	// Turn off alpha test.
	mDefaultShader->SetUniformFloat("uAlphaTest", 0.0f);
	
	// Next, translucent rendering.
	glEnable(GL_BLEND); // do alpha blending
	glDepthMask(GL_FALSE); // don't write to the depth buffer
	
	/*
	// CK: There is currently no "translucent" 3D world rendering in GK3. It is all opaque or alpha test!
	// Translucent rendering is used for the UI though.
	
	// Render translucent BSP (front-to-back).
	if(mBSP != nullptr)
	{
		mBSP->RenderTranslucent(mCamera->GetOwner()->GetPosition());
	}
	
	// TODO: Maybe need to do some sort of depth sorting for translucent rendering?
	std::sort(mMeshRenderers.begin(), mMeshRenderers.end(), [this] (const MeshRenderer* a, const MeshRenderer* b) -> bool {
		Vector3* posA = (Vector3*)a->GetMeshes()[0]->GetSubmesh(0)->GetPositions();
		Vector3* posB = (Vector3*)b->GetMeshes()[0]->GetSubmesh(0)->GetPositions();
		float distASq = (this->mCamera->GetOwner()->GetPosition() - a->GetOwner()->GetPosition()).GetLengthSq();
		float distBSq = (this->mCamera->GetOwner()->GetPosition() - b->GetOwner()->GetPosition()).GetLengthSq();
		return distASq > distBSq;
	});
	 
	// Render translucent meshes (no particular order).
	// PROBLEM: these sometimes overlap themselves AND alpha geometry in BSP.
	// To fix this, I can only see merging with BSP contents AND sorting as an option...
	for(auto& meshRenderer : mMeshRenderers)
	{
		meshRenderer->RenderTranslucent();
	}
	*/
	
	// UI is translucent rendering, BUT the view/proj matrix is different.
	mDefaultShader->SetUniformMatrix4("uViewProj", Matrix4::MakeSimpleScreenOrtho(GetWindowWidth(), GetWindowHeight()));
	
	// Don't do depth test because UI draws above everything.
	glDisable(GL_DEPTH_TEST);
	
	// Render UI elements.
	const std::vector<UICanvas*>& canvases = UICanvas::GetCanvases();
	for(auto& canvas : canvases)
	{
		canvas->Render();
	}
	
	// Switch back to opaque rendering for debug rendering.
	// Debug rendering happens after all else, so any previous function can ask for debug draws successfully.
	// Also, don't bother with depth write or depth test so debug lines aren't obfuscated!
	glDisable(GL_BLEND); // do not perform alpha blending
	
	// Gotta reset view/proj again...
	mDefaultShader->SetUniformMatrix4("uViewProj", viewProjMatrix);
	
	// Render debug elements.
	Debug::Render();
	
	// Present to screen.
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
