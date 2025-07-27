#include "Renderer.h"

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
#include "Paths.h"
#include "Profiler.h"
#include "RectTransform.h"
#include "RenderTransforms.h"
#include "SaveManager.h"
#include "SceneManager.h"
#include "SequentialFilePathGenerator.h"
#include "Skybox.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIWidget.h"

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
    if(!GAPI::Set<GAPI_OpenGL>())
    {
        return false;
    }

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
    Shader* defaultShader = gAssetManager.LoadShader("Texture", "Uber", { "FEATURE_TEXTURING" });
    if(defaultShader == nullptr) { return false; }
    Material::sDefaultShader = defaultShader;

    // Pre-load additional shaders.
    // One reason this is important is because GL commands can only run on the main thread.
    // Avoid dealing with background thread loading of shaders by loading them all up front.
    gAssetManager.LoadShader("LightmapTexture", "Uber", { "FEATURE_TEXTURING", "FEATURE_LIGHTMAPS" });
    gAssetManager.LoadShader("LitTexture", "Uber", { "FEATURE_TEXTURING", "FEATURE_LIGHTING" });
    gAssetManager.LoadShader("Skybox", "Uber", { "FEATURE_SKYBOX" });
    gAssetManager.LoadShader("TextColorReplace", "Uber", { "FEATURE_TEXTURING", "FEATURE_COLOR_REPLACE" });
    gAssetManager.LoadShader("PointsAsCircles", "Uber", { "FEATURE_TEXTURING", "FEATURE_DRAW_POINTS_AS_CIRCLES" });

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
    if(GAPI::Get() != nullptr)
    {
        GAPI::Get()->Shutdown();
    }
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
        PROFILER_BEGIN_SAMPLE("Renderer Generate Matrices");
        {
            // We'll need the projection and view matrix for the camera a few times below.
            projectionMatrix = mCamera->GetProjectionMatrix();
            viewMatrix = mCamera->GetLookAtMatrix();
        }
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Skybox");
        {
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
        }
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Opaque World Geometry");
        {
            // All opaque rendering uses alpha test and culls back faces.
            Material::UseAlphaTest(true);
            GAPI::Get()->SetPolygonCullMode(GAPI::CullMode::Back);

            // Set the view & projection matrices for normal 3D camera-oriented rendering.
            Material::SetViewMatrix(viewMatrix);
            Material::SetProjMatrix(projectionMatrix);

            PROFILER_BEGIN_SAMPLE("Render Opaque BSP");
            // Render BSP before normal mesh renderers, since it has a tendency to completely cover most of the screen.
            // This can help reduce overdraw.
            if(mBSP != nullptr)
            {
                mBSP->RenderOpaque(mCamera->GetOwner()->GetPosition(), mCamera->GetOwner()->GetForward());
            }
            PROFILER_END_SAMPLE();

            PROFILER_BEGIN_SAMPLE("Render Opaque Meshes");
            // Render opaque meshes (no particular order).
            // Sorting is probably not worthwhile b/c BSP likely mostly filled the z-buffer at this point.
            // And with the z-buffer, we can render opaque meshed correctly regardless of order.
            for(MeshRenderer* meshRenderer : mMeshRenderers)
            {
                meshRenderer->Render(true, false);
            }
            PROFILER_END_SAMPLE();
        }
        PROFILER_END_SAMPLE();

        PROFILER_BEGIN_SAMPLE("Render Translucent World Geometry");
        {
            // For translucent rendering, we don't use alpha test.
            // We also don't want to cull polygons.
            Material::UseAlphaTest(false);
            GAPI::Get()->SetPolygonCullMode(GAPI::CullMode::None);

            GAPI::Get()->SetBlendEnabled(true);       // enable blending
            GAPI::Get()->SetDepthWriteEnabled(false); // don't write to depth buffer
            GAPI::Get()->SetDepthTestEnabled(true);   // do still test depth buffer

            // Currently, all translucent rendering in the world is lightmaps/shadows/decals. These work great with "Multiply" blend mode.
            //TODO: This seems like a bad global assumption to make, and perhaps the blend mode should be specified by the material.
            GAPI::Get()->SetBlendMode(GAPI::BlendMode::Multiply);

            PROFILER_BEGIN_SAMPLE("Render Translucent BSP");
            // Render translucent BSP (only lightmaps currently).
            if(mBSP != nullptr)
            {
                mBSP->RenderTranslucent();
            }
            PROFILER_END_SAMPLE();

            PROFILER_BEGIN_SAMPLE("Render Translucent Meshes");
            // Render all translucent meshes.
            //TODO: Probably about 99% of these meshes are opaque only; it seems a bit wasteful to iterate them again. But maybe it's OK?
            for(MeshRenderer* meshRenderer : mMeshRenderers)
            {
                meshRenderer->Render(false, true);
            }
            PROFILER_END_SAMPLE();
        }
        PROFILER_END_SAMPLE();
    }

    PROFILER_BEGIN_SAMPLE("Render UI");
    {
        // UI RENDERING (TRANSLUCENT)
        GAPI::Get()->SetBlendEnabled(true);       // enable blending
        GAPI::Get()->SetDepthWriteEnabled(false); // don't write to depth buffer
        GAPI::Get()->SetDepthTestEnabled(false);  // no depth test b/c UI draws over everything

        // UI uses proper alpha blending.
        GAPI::Get()->SetBlendMode(GAPI::BlendMode::AlphaBlend);

        // UI uses a view/proj setup for now - world space for UI maps to pixel size of screen.
        // Bottom-left corner of screen is origin, +x is right, +y is up.
        Material::SetViewMatrix(Matrix4::Identity);
        Material::SetProjMatrix(RenderTransforms::MakeOrthoBottomLeft(static_cast<float>(Window::GetWidth()), static_cast<float>(Window::GetHeight())));

        // Render UI elements.
        // Any renderable UI element is contained within a Canvas.
        UICanvas::RenderCanvases();
    }
    PROFILER_END_SAMPLE();

    PROFILER_BEGIN_SAMPLE("Render Debug");
    {
        // OPAQUE DEBUG RENDERING
        // Switch back to opaque rendering for debug rendering.
        // Debug rendering happens after all else, so any previous function can ask for debug draws successfully.
        GAPI::Get()->SetBlendEnabled(false); // do opaque rendering

        // If depth test is enabled, debug visualizations will be obscured by geometry.
        // This is sometimes helpful, sometimes not...so toggle it on/off as needed.
        GAPI::Get()->SetDepthTestEnabled(!Debug::GetFlag("DrawDebugShapesOverGeometry"));

        // Gotta reset view/proj again...
        if(mCamera != nullptr)
        {
            Material::SetViewMatrix(viewMatrix);
            Material::SetProjMatrix(projectionMatrix);
        }

        #if defined(_DEBUG)
        // Render an axis at the world origin.
        Debug::DrawAxes(Vector3::Zero);
        #endif

        // Render debug elements.
        // Any debug commands from earlier are queued internally, and only drawn when this is called!
        Debug::Render();
    }
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
    std::vector<UIWidget*> widgets;
    for(auto& actor : gSceneManager.GetActors())
    {
        if(actor->GetTransform()->GetParent() == nullptr && actor->GetTransform()->IsA<RectTransform>())
        {
            actor->GetTransform()->SetDirty();

            // Likewise, some UI widgets may rely on window size for positioning/presentation.
            // For example, a UILabel that is in a RectTransform that stretches to the window size.
            // Notify UI widgets that the window size has changed, so they should recalculate too.
            actor->GetComponents<UIWidget>(widgets, true);
            for(UIWidget* widget : widgets)
            {
                widget->SetDirty();
            }
        }
    }
}

Texture* Renderer::TakeScreenshotToTexture() const
{
    // Create texture with window size.
    Texture* texture = new Texture(Window::GetWidth(), Window::GetHeight());

    // Get the screen pixels into the texture.
    GAPI::Get()->GetScreenPixels(Window::GetWidth(), Window::GetHeight(), texture->GetPixelData());

    // At least in OpenGL, the texture data is from bottom-left, but our texture class assumes top-left.
    // Flipping vertically resolves that.
    texture->FlipVertically();
    return texture;
}

void Renderer::TakeScreenshotToFile() const
{
    static SequentialFilePathGenerator pathGenerator(Paths::GetUserDataPath("Screenshots"), "screenshot_%03d.png");

    // Take a screenshot to texture.
    Texture* screenshot = TakeScreenshotToTexture();

    // Write the texture to the next available screenshot file.
    screenshot->WriteToFile(pathGenerator.GenerateFilePath(true));

    // No longer need the texture - delete it.
    delete screenshot;
}