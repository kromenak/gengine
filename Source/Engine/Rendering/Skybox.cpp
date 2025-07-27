#include "Skybox.h"

#include "Actor.h"
#include "AssetManager.h"
#include "Camera.h"
#include "Collisions.h"
#include "GAPI.h"
#include "Mesh.h"
#include "Ray.h"
#include "Renderer.h"
#include "Texture.h"

namespace
{
    const float kSkyboxSize = 20.0f;
    float points[] = {
        // Back
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Top-left
        -kSkyboxSize, -kSkyboxSize, -kSkyboxSize, // Bottom-left
         kSkyboxSize, -kSkyboxSize, -kSkyboxSize, // Bottom-right
         kSkyboxSize, -kSkyboxSize, -kSkyboxSize, // Bottom-right
         kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Top-right
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Top-left

        // Left
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
        -kSkyboxSize, -kSkyboxSize, -kSkyboxSize,
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize,
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize,
        -kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,

        // Right
        kSkyboxSize, -kSkyboxSize, -kSkyboxSize,
        kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
        kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
        kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
        kSkyboxSize,  kSkyboxSize, -kSkyboxSize,
        kSkyboxSize, -kSkyboxSize, -kSkyboxSize,

        // Front
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
        -kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
         kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
         kSkyboxSize,  kSkyboxSize,  kSkyboxSize,
         kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,

        // Top
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Back-left
         kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Back-right
         kSkyboxSize,  kSkyboxSize,  kSkyboxSize, // Front-right
         kSkyboxSize,  kSkyboxSize,  kSkyboxSize, // Front-right
        -kSkyboxSize,  kSkyboxSize,  kSkyboxSize, // Front-left
        -kSkyboxSize,  kSkyboxSize, -kSkyboxSize, // Back-left

        // Bottom
        -kSkyboxSize, -kSkyboxSize, -kSkyboxSize,
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
         kSkyboxSize, -kSkyboxSize, -kSkyboxSize,
         kSkyboxSize, -kSkyboxSize, -kSkyboxSize,
        -kSkyboxSize, -kSkyboxSize,  kSkyboxSize,
         kSkyboxSize, -kSkyboxSize,  kSkyboxSize
    };
}

Skybox::Skybox(const SkyboxTextures& textures) : mTextures(textures)
{
    // Load any mask textures.
    for(int i = 0; i < 6; ++i)
    {
        if(textures.array[i] != nullptr)
        {
            mMaskTextures.array[i] = gAssetManager.LoadSceneTexture(textures.array[i]->GetNameNoExtension() + "_MASK", textures.array[i]->GetScope());
        }
    }
}

Skybox::~Skybox()
{
    // Delete skybox mesh.
    delete mSkyboxMesh;

    // Delete cubemap texture.
    if(mCubemapHandle != nullptr)
    {
        GAPI::Get()->DestroyCubemap(mCubemapHandle);
    }
}

void Skybox::Render()
{
    // The first time we try to render, create all rendering objects.
    // We can't do this stuff in the constructor because that might happen on a background loading thread.
    // And OpenGL, for example, wants you to do rendering tasks on the main thread only.
    if(mSkyboxMesh == nullptr)
    {
        CreateMeshAndCubemap();
    }

    // Activate the material.
    mMaterial.Activate(mRotationMatrix);

    // Activate and bind the cubemap texture.
    GAPI::Get()->ActivateCubemap(mCubemapHandle);

    // Render the skybox.
    mSkyboxMesh->Render();
}

uint8_t Skybox::Raycast(const Ray& ray)
{
    // Raycasting the skybox is different than other types of raycasts (in GK3 at least). Here's how it works:
    // 1) A ray is checked against skybox triangles for a hit.
    // 2) If a hit happens, and the side of the skybox has a hit mask assigned, we calculate the hit pixel in that mask.
    // 3) The mask is assumed to be palette-based. It's queried and the hit palette index is returned.
    // 4) Zero is returned if no hit occurs, no mask exists, or hit an unimportant part of the mask.

    // No mesh means no raycast.
    if(mSkyboxMesh == nullptr || mSkyboxMesh->GetSubmeshCount() == 0) { return 0; }

    // We can avoid doing any of this if no mask texture is defined.
    bool anyMaskTexture = false;
    for(Texture* texture : mMaskTextures.array)
    {
        if(texture != nullptr)
        {
            anyMaskTexture = true;
            break;
        }
    }
    if(!anyMaskTexture) { return 0; }

    // There is a mask, so we might hit it. Do the actual raycast logic.

    // Remember, the skybox is a 20-unit cube at (0, 0, 0) in view space.
    // To get a ray from "world space" to "view space", we just need to subtract the camera's position from the ray's position.
    Vector3 localRayOrigin = ray.origin - gRenderer.GetCamera()->GetOwner()->GetPosition();

    // Because skyboxes can be rotated, we also need to transform the ray direction.
    Matrix4 worldToMeshMatrix = Matrix4::Inverse(mRotationMatrix);
    Vector3 localRayDirection = worldToMeshMatrix.TransformVector(ray.direction);
    localRayDirection.Normalize();

    // Create local ray.
    Ray localRay(localRayOrigin, localRayDirection);

    // Iterate and check for ray/triangle intersection on all sides of the skybox cube.
    // Each side of the cube is made up of two triangles.
    float t = FLT_MAX;
    float u = 0.0f;
    float v = 0.0f;
    for(uint32_t i = 0; i < mSkyboxMesh->GetSubmesh(0)->GetVertexCount(); i += 3)
    {
        Vector3 vert1 = mSkyboxMesh->GetSubmesh(0)->GetVertexPosition(i);
        Vector3 vert2 = mSkyboxMesh->GetSubmesh(0)->GetVertexPosition(i + 1);
        Vector3 vert3 = mSkyboxMesh->GetSubmesh(0)->GetVertexPosition(i + 2);
        if(Intersect::TestRayTriangle(localRay, vert1, vert2, vert3, t, u, v))
        {
            // We DID hit a triangle. Get the point of the hit on the skybox cube.
            Vector3 hitPoint = localRay.GetPoint(t);
            //std::cout << "Skybox hit at " << hitPoint << std::endl;

            // When we did the Ray/Triangle intersection test, we also calculated the barycentric coordinates as a byproduct of that test.
            // We can use those here to calculate the UV coordinates associated with the ray hit point.
            Vector3 pointUV = (vert2 * u) + (vert3 * v) + (vert1 * (1.0f - u - v));

            // This normalizes the UV from (-20, 20) range to (0, 1) range.
            pointUV /= kSkyboxSize;
            pointUV += Vector3::One;
            pointUV *= 0.5f;

            // This is where things get a bit wonky. We have six skybox faces.
            // We want the calculated UV to be (0,0) in bottom-left and (1,0) at top-right when facing that side.
            // But that isn't the case, so some manual manipulation is needed depending on the side.
            //TODO: There may be an elegant way to do this with a few lines of vector math.
            const float kSideDetectPrecision = 0.1f;
            Texture* maskTexture = nullptr;
            Vector2 finalUV;
            if(Math::Approximately(hitPoint.x, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Left\n");
                finalUV = Vector2(pointUV.z, pointUV.y);
                maskTexture = mMaskTextures.named.left;
            }
            else if(Math::Approximately(hitPoint.x, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Right\n");
                finalUV = Vector2(1.0f - pointUV.z, pointUV.y);
                maskTexture = mMaskTextures.named.right;
            }
            else if(Math::Approximately(hitPoint.z, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Back\n");
                finalUV = Vector2(1.0f - pointUV.x, pointUV.y);
                maskTexture = mMaskTextures.named.front;
            }
            else if(Math::Approximately(hitPoint.z, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Front\n");
                finalUV = Vector2(pointUV.x, pointUV.y);
                maskTexture = mMaskTextures.named.back;
            }
            else if(Math::Approximately(hitPoint.y, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Down\n");
                finalUV = Vector2(1.0f - pointUV.z, pointUV.x);
                maskTexture = mMaskTextures.named.bottom;
            }
            else if(Math::Approximately(hitPoint.y, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Up\n");
                finalUV = Vector2(1.0f - pointUV.z, 1.0f - pointUV.x);
                maskTexture = mMaskTextures.named.top;
            }

            // Ok, our raycast did hit a side of the skybox, and we've determined which side.
            // If this side had a mask texture, we now need to check the mask texture to get a palette index.
            if(maskTexture != nullptr)
            {
                // UVs are bottom-left to top-right, but Texture class uses pixel coordinates from top-left to bottom-right.
                // Flip the y-axis to align this.
                finalUV.y = 1.0f - finalUV.y;

                // Convert the UV into a specific pixel position.
                Vector2 pixelPos(finalUV.x * maskTexture->GetWidth(), finalUV.y * maskTexture->GetHeight());

                // Get the palette index at that position.
                return maskTexture->GetPixelPaletteIndex(static_cast<int>(pixelPos.x), static_cast<int>(pixelPos.y));
            }

            // No mask texture, so we won't hit anything.
            return 0;
        }
    }

    // No hit apparently.
    return 0;
}

void Skybox::CreateMeshAndCubemap()
{
    // Generate the mesh used to render the skybox.
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 36);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);
        meshDefinition.AddVertexData(VertexAttribute::Position, points);
        meshDefinition.ownsData = false;

        mSkyboxMesh = new Mesh();
        mSkyboxMesh->AddSubmesh(meshDefinition);
    }

    // Use skybox shader to render the skybox.
    mMaterial.SetShader(gAssetManager.GetShader("Skybox"));

    // Generate cubemap from skybox textures.
    {
        // The caller doesn't necessarily need to provide all skybox textures.
        // For example, the "down" texture is often unneeded.

        // However, OpenGL *does* require we provide valid textures for all sides.
        // AND they require that textures have the same width/height.

        // First off, find ANY valid texture passed in. There must be at least one, or you get no skybox.
        Texture* defaultTexture = nullptr;
        for(Texture* texture : mTextures.array)
        {
            if(texture != nullptr)
            {
                defaultTexture = texture;
                break;
            }
        }
        if(defaultTexture == nullptr)
        {
            printf("Can't create skybox - no textures were provided!\n");
            return;
        }

        // Then, set any null textures equal to that texture.
        for(Texture*& texture : mTextures.array)
        {
            if(texture == nullptr)
            {
                texture = defaultTexture;
            }
        }

        // At least in OpenGL, all cubemap textures MUST be square and have the same resolution.
        // If any texture is inconsistent, adjust it to be the correct size.
        int largestSize = 0;
        for(Texture*& texture : mTextures.array)
        {
            if(texture->GetWidth() > largestSize)
            {
                largestSize = texture->GetWidth();
            }
        }
        for(Texture*& texture : mTextures.array)
        {
            if(texture->GetWidth() != largestSize)
            {
                texture->Resize(largestSize, largestSize);
            }
        }

        // Build cubemap params struct.
        GAPI::CubemapParams params;
        params.left.width = mTextures.named.left->GetWidth();
        params.left.height = mTextures.named.left->GetHeight();
        params.left.format = mTextures.named.left->GetFormat();
        params.left.pixels = mTextures.named.left->GetPixelData();

        params.right.width = mTextures.named.right->GetWidth();
        params.right.height = mTextures.named.right->GetHeight();
        params.right.format = mTextures.named.right->GetFormat();
        params.right.pixels = mTextures.named.right->GetPixelData();

        params.back.width = mTextures.named.back->GetWidth();
        params.back.height = mTextures.named.back->GetHeight();
        params.back.format = mTextures.named.back->GetFormat();
        params.back.pixels = mTextures.named.back->GetPixelData();

        params.front.width = mTextures.named.front->GetWidth();
        params.front.height = mTextures.named.front->GetHeight();
        params.front.format = mTextures.named.front->GetFormat();
        params.front.pixels = mTextures.named.front->GetPixelData();

        params.bottom.width = mTextures.named.bottom->GetWidth();
        params.bottom.height = mTextures.named.bottom->GetHeight();
        params.bottom.format = mTextures.named.bottom->GetFormat();
        params.bottom.pixels = mTextures.named.bottom->GetPixelData();

        params.top.width = mTextures.named.top->GetWidth();
        params.top.height = mTextures.named.top->GetHeight();
        params.top.format = mTextures.named.top->GetFormat();
        params.top.pixels = mTextures.named.top->GetPixelData();

        // Create cubemap from params.
        mCubemapHandle = GAPI::Get()->CreateCubemap(params);
    }
}
