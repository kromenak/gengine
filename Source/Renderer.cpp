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

float full_quad_vertices[] = {
	-1.0f,  1.0f, 0.0f, // upper-left
	1.0f,  1.0f, 0.0f, // upper-right
	1.0f, -1.0f, 0.0f, // lower-right
	-1.0f, -1.0f, 0.0f, // lower-left
};

float full_quad_uvs[] = {
	0.0f, 1.0f,		// upper-left
	1.0f, 1.0f,		// upper-right
	1.0f, 0.0f,		// lower-right
	0.0f, 0.0f		// lower-left
};

Mesh* fullQuad = nullptr;

float ui_quad_vertices[] = {
	0.0f,  1.0f, 0.0f, // upper-left
	1.0f,  1.0f, 0.0f, // upper-right
	1.0f,  0.0f, 0.0f, // lower-right
	0.0f,  0.0f, 0.0f, // lower-left
};

float ui_quad_uvs[] = {
	0.0f, 0.0f,		// upper-left
	1.0f, 0.0f,		// upper-right
	1.0f, 1.0f,		// lower-right
	0.0f, 1.0f		// lower-left
};

Mesh* uiQuad = nullptr;

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
	Shader* defaultShader = Services::GetAssets()->LoadShader("3D-Diffuse-Tex");
	if(defaultShader == nullptr) { return false; }
	Material::sDefaultShader = defaultShader;
	
    // Load skybox shader and create material.
    Shader* skyboxShader = Services::GetAssets()->LoadShader("3D-Skybox");
    if(skyboxShader == nullptr) { return false; }
	mSkyboxMaterial.SetShader(skyboxShader);
	
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
	
	// Perhaps errantly, the normal "quad" doesn't have "full" vertex positons (using 1 instead of 0.5)
	// Also, the UVs seemed to be upside down?
	// Anyway, this full quad seems to work a lot better for render texture usage...perhaps I'll need to fix the quad at some point.
	fullQuad = new Mesh(4, 5 * sizeof(float), MeshUsage::Static);
	Submesh* fullQuadSubmesh = fullQuad->GetSubmesh(0);
	fullQuadSubmesh->SetRenderMode(RenderMode::Triangles);
	fullQuadSubmesh->SetPositions(full_quad_vertices);
	fullQuadSubmesh->SetUV1(full_quad_uvs);
	fullQuadSubmesh->SetIndexes(quad_indices, 6);
	
	uiQuad = new Mesh(4, 5 * sizeof(float), MeshUsage::Static);
	Submesh* uiQuadSubmesh = uiQuad->GetSubmesh(0);
	uiQuadSubmesh->SetRenderMode(RenderMode::Triangles);
	uiQuadSubmesh->SetPositions(ui_quad_vertices);
	uiQuadSubmesh->SetUV1(ui_quad_uvs);
	uiQuadSubmesh->SetIndexes(quad_indices, 6);
	
    // Init succeeded!
    return true;
}

void Renderer::Shutdown()
{
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Renderer::Render()
{
	// Enable opaque rendering (no blend, write to & test depth buffer).
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
	
	// We'll need the projection and view matrix for the camera a few times below.
	Matrix4 projectionMatrix = mCamera->GetProjectionMatrix();
	Matrix4 viewMatrix = mCamera->GetLookAtMatrix();
	
	// STEP 1: SKYBOX RENDERING
	// Draw the skybox first, which is just a little cube around the camera.
	// Don't write to depth mask, or else you can ONLY see skybox (b/c again, little cube).
	glDepthMask(GL_FALSE); // stops writing to depth buffer
	if(mSkybox != nullptr)
	{
		// To get the "infinite distance" skybox effect, we need to use a look-at
		// matrix that doesn't take the camera's position into account.
		Material::SetViewMatrix(mCamera->GetLookAtMatrixNoTranslate());
		Material::SetProjMatrix(projectionMatrix);
		mSkybox->Render();
	}
	glDepthMask(GL_TRUE); // start writing to depth buffer
	
	// STEP 2: OPAQUE WORLD RENDERING
	// STEP 2A: OPAQUE BSP RENDERING
	// Set the view & projection matrices for normal 3D camera-oriented rendering.
	Material::SetViewMatrix(viewMatrix);
	Material::SetProjMatrix(projectionMatrix);
	
	// If BSP was using a material, we could just use Material function here.
	// But we must also set on default shader for BSP...for now.
	Material::UseAlphaTest(true);
	
	// Render opaque BSP (front-to-back).
	if(mBSP != nullptr)
	{
		mBSP->RenderOpaque(mCamera->GetOwner()->GetPosition());
	}
	
	// STEP 2B: OPAQUE MESH RENDERING
	// Render opaque meshes (no particular order).
	// Sorting is probably not worthwhile b/c BSP likely mostly filled the z-buffer at this point.
	// And with the z-buffer, we can render opaque meshed correctly regardless of order.
	for(auto& meshRenderer : mMeshRenderers)
	{
		meshRenderer->RenderOpaque();
	}
	
	// Turn off alpha test.
	Material::UseAlphaTest(false);
	
	// STEP 3: TRANSLUCENT WORLD RENDERING
	// So far, GK3 doesn't seem to have any translucent geometry AT ALL!
	// Everything is either opaque or alpha test.
	// If we DO need translucent rendering, it probably can only be meshes or BSP, but not both.
	
	// STEP 4: (TRANSLUCENT) UI RENDERING
	glEnable(GL_BLEND); // do alpha blending
	glDepthMask(GL_FALSE); // don't write to the depth buffer
	
	// UI uses a view/proj setup for now - world space for UI maps to pixel size of screen.
	// Bottom-left corner of screen is origin, +x is right, +y is up.
	Material::SetViewMatrix(Matrix4::Identity);
	Material::SetProjMatrix(Matrix4::MakeSimpleScreenOrtho(static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight())));
	
	// Don't do depth test because UI draws above everything.
	// This means UI is basically painter's algorithm though!
	glDisable(GL_DEPTH_TEST);
	
	// Render UI elements.
	// Any renderable UI element is contained within a Canvas.
	const std::vector<UICanvas*>& canvases = UICanvas::GetCanvases();
	for(auto& canvas : canvases)
	{
		canvas->Render();
	}
	
	// STEP 5: OPAQUE DEBUG RENDERING
	// Switch back to opaque rendering for debug rendering.
	// Debug rendering happens after all else, so any previous function can ask for debug draws successfully.
	// Also, don't bother with depth write or depth test so debug lines aren't obfuscated!
	glDisable(GL_BLEND); // do not perform alpha blending
	
	// Gotta reset view/proj again...
	Material::SetViewMatrix(viewMatrix);
	Material::SetProjMatrix(projectionMatrix);
	
	// Render an axis at the world origin, for debugging.
	Debug::DrawAxes(Vector3::Zero);
	
	// Render debug elements.
	// Any debug commands from earlier are queued internally, and only drawn when this is called!
	Debug::Render();
	
	// STEP 6: ALL DONE! PUT THE ART ON DISPLAY!
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

void Renderer::SetSkybox(Skybox* skybox)
{
	mSkybox = skybox;
	if(mSkybox != nullptr)
	{
		mSkybox->SetMaterial(mSkyboxMaterial);
	}
}
