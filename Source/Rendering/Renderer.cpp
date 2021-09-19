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
#include "Profiler.h"
#include "RenderTransforms.h"
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
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f
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
    mWindow = SDL_CreateWindow("Gabriel Knight 3", 100, 100, mScreenWidth, mScreenHeight, SDL_WINDOW_OPENGL);
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
    
    #if VIEW_HAND == VIEW_LH
    // We can use left-hand or right-hand view space, but GL's depth buffer defaults assume right-hand.
    // If using left-hand, we essentially "flip" the depth buffer.
    // Clear to 0 (instead of 1) and use GL_GREATER for depth tests (rather than GL_LESS).
    glClearDepth(0);
    glDepthFunc(GL_GREATER);
    #endif
	
    // Load default shader.
	Shader* defaultShader = Services::GetAssets()->LoadShader("3D-Tex");
	if(defaultShader == nullptr) { return false; }
	Material::sDefaultShader = defaultShader;
	
    // Load skybox shader and create material.
    Shader* skyboxShader = Services::GetAssets()->LoadShader("3D-Skybox");
    if(skyboxShader == nullptr) { return false; }
	mSkyboxMaterial.SetShader(skyboxShader);
    
    MeshDefinition meshDefinition;
    meshDefinition.meshUsage = MeshUsage::Static;
    
    meshDefinition.vertexDefinition.layout = VertexDefinition::Layout::Packed;
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::Position);
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::Color);
    std::vector<float*> vertexData;
    
	// Create line mesh, which is helpful for debugging.
    vertexData.push_back(line_vertices);
    vertexData.push_back(line_colors);
    
    meshDefinition.vertexCount = 2;
    meshDefinition.vertexData = &vertexData[0];
    
	line = new Mesh();
    Submesh* lineSubmesh = line->AddSubmesh(meshDefinition);
	lineSubmesh->SetRenderMode(RenderMode::Lines);
	
    // Create axes mesh, which is helpful for debugging.
    meshDefinition.vertexCount = 6;
    vertexData[0] = axis_vertices;
    vertexData[1] = axis_colors;
    
    axes = new Mesh();
    Submesh* axesSubmesh = axes->AddSubmesh(meshDefinition);
    axesSubmesh->SetRenderMode(RenderMode::Lines);
	
	// Create quad mesh, which is used for UI and 2D rendering.
    meshDefinition.vertexDefinition.attributes[1] = VertexAttribute::UV1;
    meshDefinition.vertexCount = 4;
    vertexData[0] = quad_vertices;
    vertexData[1] = quad_uvs;
    meshDefinition.indexCount = 6;
    meshDefinition.indexData = quad_indices;
    
	quad = new Mesh();
    Submesh* quadSubmesh = quad->AddSubmesh(meshDefinition);
	quadSubmesh->SetRenderMode(RenderMode::Triangles);
	
	// Perhaps errantly, the normal "quad" doesn't have "full" vertex positons (using 1 instead of 0.5)
	// Also, the UVs seemed to be upside down?
	// Anyway, this full quad seems to work a lot better for render texture usage...perhaps I'll need to fix the quad at some point.
    meshDefinition.vertexCount = 4;
    vertexData[0] = full_quad_vertices;
    vertexData[1] = full_quad_uvs;
    
	fullQuad = new Mesh();
    Submesh* fullQuadSubmesh = fullQuad->AddSubmesh(meshDefinition);
	fullQuadSubmesh->SetRenderMode(RenderMode::Triangles);
	
    // And a quad for UI usage...
    meshDefinition.vertexCount = 4;
    vertexData[0] = ui_quad_vertices;
    vertexData[1] = ui_quad_uvs;
    
	uiQuad = new Mesh();
    Submesh* uiQuadSubmesh = uiQuad->AddSubmesh(meshDefinition);
	uiQuadSubmesh->SetRenderMode(RenderMode::Triangles);
    
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
    PROFILER_BEGIN_SAMPLE("Renderer Clear");
    // Enable opaque rendering (no blend, write to & test depth buffer).
    // Do this BEFORE clear to avoid some glitchy graphics.
    glDisable(GL_BLEND); // do not perform alpha blending (opaque rendering)
    glDepthMask(GL_TRUE); // start writing to depth buffer
    glEnable(GL_DEPTH_TEST); // do depth comparisons and update the depth buffer

    // Clear color and depth buffers from last frame.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    PROFILER_END_SAMPLE();
	
	// Render camera-oriented stuff.
    Matrix4 projectionMatrix;
    Matrix4 viewMatrix;
	if(mCamera != nullptr)
	{
        PROFILER_BEGIN_SAMPLE("Renderer Generate Matrices")
        // We'll need the projection and view matrix for the camera a few times below.
        projectionMatrix = mCamera->GetProjectionMatrix();
        viewMatrix = mCamera->GetLookAtMatrix();
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Skybox");
        // SKYBOX RENDERING
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
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render BSP");
        // OPAQUE BSP RENDERING
        // All opaque world rendering uses alpha test.
        Material::UseAlphaTest(true);
        
        // Set the view & projection matrices for normal 3D camera-oriented rendering.
        Material::SetViewMatrix(viewMatrix);
        Material::SetProjMatrix(projectionMatrix);

        // Render opaque BSP. This should occur front-to-back, which has no overdraw.
        if(mBSP != nullptr)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT); //TODO: Why does this need to be FRONT? Is that the way BSP data is authored, or is my winding order wrong?
            mBSP->RenderOpaque(mCamera->GetOwner()->GetPosition(), mCamera->GetOwner()->GetForward());
            glDisable(GL_CULL_FACE);
        }
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Objects");
        // OPAQUE MESH RENDERING
        // Render opaque meshes (no particular order).
        // Sorting is probably not worthwhile b/c BSP likely mostly filled the z-buffer at this point.
        // And with the z-buffer, we can render opaque meshed correctly regardless of order.
        for(auto& meshRenderer : mMeshRenderers)
        {
            meshRenderer->Render();
        }
        
        // Turn off alpha test.
        Material::UseAlphaTest(false);
        PROFILER_END_SAMPLE();
        
        // TRANSLUCENT WORLD RENDERING
        // So far, GK3 doesn't seem to have any translucent geometry AT ALL!
        // Everything is either opaque or alpha test.
        // If we DO need translucent rendering, it probably can only be meshes or BSP, but not both.
        //TODO: Any translucent world rendering.
    }

    PROFILER_BEGIN_SAMPLE("Render UI");
    // UI RENDERING (TRANSLUCENT)
    glEnable(GL_BLEND); // do alpha blending
    glDepthMask(GL_FALSE); // don't write to the depth buffer
    glDisable(GL_DEPTH_TEST); // no depth test b/c UI draws over everything
    
    // UI uses a view/proj setup for now - world space for UI maps to pixel size of screen.
    // Bottom-left corner of screen is origin, +x is right, +y is up.
    Material::SetViewMatrix(Matrix4::Identity);
    Material::SetProjMatrix(RenderTransforms::MakeOrthoBottomLeft(static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight())));
    
    // Render UI elements.
    // Any renderable UI element is contained within a Canvas.
    const std::vector<UICanvas*>& canvases = UICanvas::GetCanvases();
    for(auto& canvas : canvases)
    {
        canvas->Render();
    }
    PROFILER_END_SAMPLE();

    PROFILER_BEGIN_SAMPLE("Render Debug");
    // OPAQUE DEBUG RENDERING
    // Switch back to opaque rendering for debug rendering.
    // Debug rendering happens after all else, so any previous function can ask for debug draws successfully.
    glDisable(GL_BLEND); // do opaque rendering

    // If depth test is enabled, debug visualizations will be obscured by geometry.
    // This is sometimes helpful, sometimes not...so toggle it on/off as needed.
    glEnable(GL_DEPTH_TEST);

    // Gotta reset view/proj again...
    Material::SetViewMatrix(viewMatrix);
    Material::SetProjMatrix(projectionMatrix);
    
    // Render an axis at the world origin.
    Debug::DrawAxes(Vector3::Zero);
    
    // Render debug elements.
    // Any debug commands from earlier are queued internally, and only drawn when this is called!
    Debug::Render();
    PROFILER_END_SAMPLE();
    
	// Present to window.
    PROFILER_BEGIN_SAMPLE("Renderer Present");
	SDL_GL_SwapWindow(mWindow);
    PROFILER_END_SAMPLE();
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

void Renderer::ToggleFullscreen()
{
    bool isFullscreen = SDL_GetWindowFlags(mWindow) & SDL_WINDOW_FULLSCREEN;
    SDL_SetWindowFullscreen(mWindow, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}