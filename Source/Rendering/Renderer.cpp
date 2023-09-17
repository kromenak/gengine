#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Actor.h"
#include "AssetManager.h"
#include "BSP.h"
#include "Camera.h"
#include "Debug.h"
#include "GAPI.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Profiler.h"
#include "RenderTransforms.h"
#include "SceneManager.h"
#include "SaveManager.h"
#include "Shader.h"
#include "Skybox.h"
#include "Texture.h"
#include "UICanvas.h"

#include "OpenGL/GAPI_OpenGL.h"

// Line
float line_vertices[] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f
};
float line_colors[] = {
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};
Mesh* line = nullptr;

// Axis
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

// UI Quad
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

Renderer gRenderer;

bool Renderer::Initialize()
{
    TIMER_SCOPED("Renderer::Initialize");

    // Create the game window.
    Window::Create("Gabriel Knight 3");
    if(Window::Get() == nullptr)
    {
        printf("Failed to create game window!\n");
        return false;
    }

    // Set which graphics API to use.
    GAPI::Set<GAPI_OpenGL>();

    // Make sure the viewport is set to the full window size.
    GAPI::Get()->SetViewport(0, 0, Window::GetWidth(), Window::GetHeight());

    // Graphics APIs are inconsistent with their conventions about whether view space should be left or right-handed.
    // However, we can configure the graphics API to use whichever approach we prefer by altering their defaults.
    #if VIEW_HAND == VIEW_LH
    GAPI::Get()->SetViewSpaceHandedness(GAPI::Handedness::LeftHand);
    #else
    GAPI::Get()->SetViewSpaceHandedness(GAPI::Handedness:RightHand);
    #endif

    // GK3 vertex data for models & BSP use a clockwise winding order.
    // However, note that *indexes* are counter-clockwise...but that doesn't seem to affect how the data is interpreted?
    GAPI::Get()->SetPolygonWindingOrder(GAPI::WindingOrder::Clockwise);
	
    // Load default shader.
	Shader* defaultShader = gAssetManager.LoadShader("3D-Tex");
	if(defaultShader == nullptr) { return false; }
	Material::sDefaultShader = defaultShader;

    // Pre-load additional shaders.
    // One reason this is important is because GL commands can only run on the main thread.
    // Avoid dealing with background thread loading of shaders by loading them all up front.
    std::string shaders[] = {
        "3D-Lightmap",
        "3D-Tex-Lit",
        "3D-Skybox"
    };
    for(auto& shader : shaders)
    {
        //printf("Load %s\n", shader.c_str());
        gAssetManager.LoadShader(shader);
    }
    gAssetManager.LoadShader("3D-Tex", "UI-Text-ColorReplace");

    // Create simple shapes (useful for debugging/visualization).
    // Line
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 2);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);

        meshDefinition.AddVertexData(VertexAttribute::Position, line_vertices);
        meshDefinition.AddVertexData(VertexAttribute::Color, line_colors);

        line = new Mesh();
        Submesh* lineSubmesh = line->AddSubmesh(meshDefinition);
        lineSubmesh->SetRenderMode(RenderMode::Lines);
    }

    // Axes
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 6);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);

        meshDefinition.AddVertexData(VertexAttribute::Position, axis_vertices);
        meshDefinition.AddVertexData(VertexAttribute::Color, axis_colors);

        axes = new Mesh();
        Submesh* axesSubmesh = axes->AddSubmesh(meshDefinition);
        axesSubmesh->SetRenderMode(RenderMode::Lines);
    }
    
    // Quad
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 4);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);

        meshDefinition.AddVertexData(VertexAttribute::Position, quad_vertices);
        meshDefinition.AddVertexData(VertexAttribute::UV1, quad_uvs);
        meshDefinition.SetIndexData(6, quad_indices);

        quad = new Mesh();
        Submesh* quadSubmesh = quad->AddSubmesh(meshDefinition);
        quadSubmesh->SetRenderMode(RenderMode::Triangles);
    }
    
	// UI Quad
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 4);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);

        meshDefinition.AddVertexData(VertexAttribute::Position, ui_quad_vertices);
        meshDefinition.AddVertexData(VertexAttribute::UV1, ui_quad_uvs);
        meshDefinition.SetIndexData(6, quad_indices);

        uiQuad = new Mesh();
        Submesh* uiQuadSubmesh = uiQuad->AddSubmesh(meshDefinition);
        uiQuadSubmesh->SetRenderMode(RenderMode::Triangles);
    }

    // Do a single render here to make sure the window is cleared/empty.
    // Because this is so early in the engine init process, this will basically just set clear color and present. 
    Render();

    // Load rendering prefs.
    mUseMipmaps = gSaveManager.GetPrefs()->GetBool(PREFS_HARDWARE_RENDERER, PREFS_MIPMAPS, true);
    mUseTrilinearFiltering = gSaveManager.GetPrefs()->GetBool(PREFS_HARDWARE_RENDERER, PREFS_TRILINEAR_FILTERING, true);

    // Init succeeded!
    return true;
}

void Renderer::Shutdown()
{
    GAPI::Get()->Shutdown();
    Window::Destroy();
}

void Renderer::Clear()
{
    PROFILER_BEGIN_SAMPLE("Renderer Clear");

    // Enable opaque rendering (no blend, write to & test depth buffer).
    // Do this BEFORE clear to avoid some glitchy graphics.
    GAPI::Get()->SetBlendEnabled(false);
    GAPI::Get()->SetDepthWriteEnabled(true);
    GAPI::Get()->SetDepthTestEnabled(true);

    // Clear color and depth buffers from last frame.
    GAPI::Get()->Clear(Color32::Black);
    PROFILER_END_SAMPLE();
}

void Renderer::Render()
{
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
        GAPI::Get()->SetDepthWriteEnabled(false);
        if(mSkybox != nullptr)
        {
            // To get the "infinite distance" skybox effect, we need to use a look-at
            // matrix that doesn't take the camera's position into account.
            Material::SetViewMatrix(mCamera->GetLookAtMatrixNoTranslate());
            Material::SetProjMatrix(projectionMatrix);
            mSkybox->Render();
        }
        GAPI::Get()->SetDepthWriteEnabled(true);
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render BSP");
        // OPAQUE BSP RENDERING
        // All opaque world rendering uses alpha test.
        Material::UseAlphaTest(true);
        GAPI::Get()->SetPolygonCullMode(GAPI::CullMode::Back);
        
        // Set the view & projection matrices for normal 3D camera-oriented rendering.
        Material::SetViewMatrix(viewMatrix);
        Material::SetProjMatrix(projectionMatrix);

        // Render opaque BSP. This should occur front-to-back, which has no overdraw.
        if(mBSP != nullptr)
        {
            mBSP->RenderOpaque(mCamera->GetOwner()->GetPosition(), mCamera->GetOwner()->GetForward());
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
        GAPI::Get()->SetPolygonCullMode(GAPI::CullMode::None);
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Translucent BSP");
        // TRANSLUCENT WORLD RENDERING
        GAPI::Get()->SetBlendEnabled(true);       // enable blending
        GAPI::Get()->SetDepthWriteEnabled(false); // don't write to depth buffer
        GAPI::Get()->SetDepthTestEnabled(true);   // do still test depth buffer

        // Currently all translucent BSP are shadow textures or decals, which look great with Multiply blend mode.
        GAPI::Get()->SetBlendMode(GAPI::BlendMode::Multiply); 
        if(mBSP != nullptr)
        {
            mBSP->RenderTranslucent();
        }
        PROFILER_END_SAMPLE();
    }

    PROFILER_BEGIN_SAMPLE("Render UI");
    // UI RENDERING (TRANSLUCENT)
    GAPI::Get()->SetBlendEnabled(true);       // enable blending
    GAPI::Get()->SetDepthWriteEnabled(false); // don't write to depth buffer
    GAPI::Get()->SetDepthTestEnabled(false);  // no depth test b/c UI draws over everything
    GAPI::Get()->SetBlendMode(GAPI::BlendMode::AlphaBlend);

    // UI uses a view/proj setup for now - world space for UI maps to pixel size of screen.
    // Bottom-left corner of screen is origin, +x is right, +y is up.
    Material::SetViewMatrix(Matrix4::Identity);
    Material::SetProjMatrix(RenderTransforms::MakeOrthoBottomLeft(static_cast<float>(Window::GetWidth()), static_cast<float>(Window::GetHeight())));
    
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
    GAPI::Get()->SetBlendEnabled(false); // do opaque rendering

    // If depth test is enabled, debug visualizations will be obscured by geometry.
    // This is sometimes helpful, sometimes not...so toggle it on/off as needed.
    GAPI::Get()->SetDepthTestEnabled(true);

    // Gotta reset view/proj again...
    Material::SetViewMatrix(viewMatrix);
    Material::SetProjMatrix(projectionMatrix);

    #if defined(_DEBUG)
    // Render an axis at the world origin.
    Debug::DrawAxes(Vector3::Zero);
    #endif
    
    // Render debug elements.
    // Any debug commands from earlier are queued internally, and only drawn when this is called!
    Debug::Render();
    PROFILER_END_SAMPLE();
}

void Renderer::Present()
{
    // Present to window.
    PROFILER_BEGIN_SAMPLE("Renderer Present");
    GAPI::Get()->Present();
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
}

void Renderer::SetUseMipmaps(bool useMipmaps)
{
    mUseMipmaps = useMipmaps;
    gSaveManager.GetPrefs()->Set(PREFS_HARDWARE_RENDERER, PREFS_MIPMAPS, mUseMipmaps);

    // Dynamically update loaded textures to use mipmaps.
    for(auto& entry : gAssetManager.GetLoadedTextures())
    {
        // The trick is that this map has both UI and scene textures. And we only want to modify *scene* textures.
        // We can look at the current filtering setting as an indicator.
        Texture* texture = entry.second;
        if(texture->GetFilterMode() != Texture::FilterMode::Point)
        {
            texture->SetMipmaps(mUseMipmaps);
        }
    }
}

void Renderer::SetUseTrilinearFiltering(bool useTrilinearFiltering)
{
    mUseTrilinearFiltering = useTrilinearFiltering;
    gSaveManager.GetPrefs()->Set(PREFS_HARDWARE_RENDERER, PREFS_TRILINEAR_FILTERING, mUseTrilinearFiltering);

    // Dynamically update loaded textures to use trilinear filtering.
    for(auto& entry : gAssetManager.GetLoadedTextures())
    {
        // The trick is that this map has both UI and scene textures. And we only want to modify *scene* textures.
        // We can look at the current filtering setting as an indicator.
        Texture* texture = entry.second;
        if(texture->GetFilterMode() != Texture::FilterMode::Point)
        {
            texture->SetFilterMode(mUseTrilinearFiltering ? Texture::FilterMode::Trilinear : Texture::FilterMode::Bilinear);
        }
    }
}

void Renderer::ChangeResolution(const Window::Resolution& resolution)
{
    Window::SetResolution(resolution);
    
    // Change the viewport to match the new width/height.
    // If you don't do this, the window size changes but the area rendered to doesn't change.
    GAPI::Get()->SetViewport(0, 0, resolution.width, resolution.height);

    // Because some RectTransforms may rely on the window size, we need to dirty all root RectTransforms in the scene.
    for(auto& actor : gSceneManager.GetActors())
    {
        if(actor->GetTransform()->GetParent() == nullptr &&
           actor->GetTransform()->IsTypeOf(RectTransform::GetType()))
        {
            actor->GetTransform()->SetDirty();
        }
    }
}