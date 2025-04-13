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
    // Generate submesh on the fly, if not yet generated.
    if(mSkyboxMesh == nullptr)
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 36);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);
        meshDefinition.AddVertexData(VertexAttribute::Position, points);
        meshDefinition.ownsData = false;
        
        mSkyboxMesh = new Mesh();
        mSkyboxMesh->AddSubmesh(meshDefinition);
        
        // Also use this opportunity to set the shader on the material.
        Shader* skyboxShader = gAssetManager.LoadShader("3D-Skybox");
        mMaterial.SetShader(skyboxShader);
    }

    // Generate cubemap texture, if not yet generated.
    if(mCubemapHandle == nullptr)
    {
        GAPI::CubemapParams params;
        if(mRightTexture != nullptr)
        {
            params.right.width = mRightTexture->GetWidth();
            params.right.height = mRightTexture->GetHeight();
            params.right.pixels = mRightTexture->GetPixelData();
        }
        if(mLeftTexture != nullptr)
        {
            params.left.width = mLeftTexture->GetWidth();
            params.left.height = mLeftTexture->GetHeight();
            params.left.pixels = mLeftTexture->GetPixelData();
        }

        // GK3 puts the front texture in the -Z direction, and the back texture in the +Z direction.
        // I have no idea why this is the case, but even in the original game, this is true.
        if(mFrontTexture != nullptr)
        {
            params.back.width = mFrontTexture->GetWidth();
            params.back.height = mFrontTexture->GetHeight();
            params.back.pixels = mFrontTexture->GetPixelData();
        }
        if(mBackTexture != nullptr)
        {
            params.front.width = mBackTexture->GetWidth();
            params.front.height = mBackTexture->GetHeight();
            params.front.pixels = mBackTexture->GetPixelData();
        }

        // GK3 up/down textures are rotated counter-clockwise 90 degrees.
        // Not sure why this choice was made, but it's required for skybox seams to match up.
        if(mUpTexture != nullptr)
        {
            mUpTexture->RotateCounterclockwise();

            params.top.width = mUpTexture->GetWidth();
            params.top.height = mUpTexture->GetHeight();
            params.top.pixels = mUpTexture->GetPixelData();
        }
        if(mDownTexture != nullptr)
        {
            mDownTexture->RotateCounterclockwise();

            params.bottom.width = mDownTexture->GetWidth();
            params.bottom.height = mDownTexture->GetHeight();
            params.bottom.pixels = mDownTexture->GetPixelData();
        }
        mCubemapHandle = GAPI::Get()->CreateCubemap(params);
    }
	
	// Activate the material.
    mMaterial.Activate(mRotationMatrix);
	
	// Activate and bind the cubemap texture.
    GAPI::Get()->ActivateCubemap(mCubemapHandle);

	// Render the skybox.
    mSkyboxMesh->Render();
}

void Skybox::SetRightTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mRightTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::SetLeftTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mLeftTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::SetFrontTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mFrontTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::SetBackTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mBackTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::SetUpTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mUpTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::SetDownTexture(Texture* texture)
{
	if(texture == nullptr) { return; }
	mDownTexture = texture;
	SetDefaultTexture(texture);
}

void Skybox::LoadMaskTextures()
{
    if(mRightTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mRightTexture->GetNameNoExtension() + "_MASK", mRightTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetRightMaskTexture(maskTexture);
        }
    }
    if(mLeftTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mLeftTexture->GetNameNoExtension() + "_MASK", mLeftTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetLeftMaskTexture(maskTexture);
        }
    }
    if(mFrontTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mFrontTexture->GetNameNoExtension() + "_MASK", mFrontTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetFrontMaskTexture(maskTexture);
        }
    }
    if(mBackTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mBackTexture->GetNameNoExtension() + "_MASK", mBackTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetBackMaskTexture(maskTexture);
        }
    }
    if(mUpTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mUpTexture->GetNameNoExtension() + "_MASK", mUpTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetUpMaskTexture(maskTexture);
        }
    }
    if(mDownTexture != nullptr)
    {
        Texture* maskTexture = gAssetManager.LoadSceneTexture(mDownTexture->GetNameNoExtension() + "_MASK", mDownTexture->GetScope());
        if(maskTexture != nullptr)
        {
            SetDownMaskTexture(maskTexture);
        }
    }
}

void Skybox::SetAzimuth(float radians)
{
    mRotationMatrix = Matrix4::MakeRotateY(radians);
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
    if(mLeftMaskTexture == nullptr && mRightMaskTexture == nullptr &&
       mBackMaskTexture == nullptr && mFrontMaskTexture == nullptr &&
       mDownMaskTexture == nullptr && mUpMaskTexture == nullptr)
    {
        return 0;
    }

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
            if(Math::Approximately(hitPoint.x, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Right\n");
                finalUV = Vector2(1.0f - pointUV.z, pointUV.y);
                maskTexture = mRightMaskTexture;
            }
            else if(Math::Approximately(hitPoint.x, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Left\n");
                finalUV = Vector2(pointUV.z, pointUV.y);
                maskTexture = mLeftMaskTexture;
            }
            else if(Math::Approximately(hitPoint.y, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Up\n");
                finalUV = Vector2(1.0f - pointUV.z, 1.0f - pointUV.x);
                maskTexture = mUpMaskTexture;
            }
            else if(Math::Approximately(hitPoint.y, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Down\n");
                finalUV = Vector2(1.0f - pointUV.z, pointUV.x);
                maskTexture = mDownMaskTexture;
            }
            else if(Math::Approximately(hitPoint.z, kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Front\n");
                finalUV = Vector2(pointUV.x, pointUV.y);
                maskTexture = mBackMaskTexture;
            }
            else if(Math::Approximately(hitPoint.z, -kSkyboxSize, kSideDetectPrecision))
            {
                //printf("Back\n");
                finalUV = Vector2(1.0f - pointUV.x, pointUV.y);
                maskTexture = mFrontMaskTexture;
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
                return maskTexture->GetPaletteIndex(static_cast<int>(pixelPos.x), static_cast<int>(pixelPos.y));
            }

            // No mask texture, so we won't hit anything.
            return 0;
        }
    }

    // No hit apparently.
    return 0;
}

void Skybox::SetDefaultTexture(Texture* texture)
{
	// The purpose of this is to ensure that each side of the skybox has a valid texture to use.
	// Skyboxes have some requirements (in OpenGL, at least) about all sides being same size, mipmap level, and format.
	// When any one side is set, we just make sure that ALL sides are set, if still null.
	if(mRightTexture == nullptr) { mRightTexture = texture; }
	if(mLeftTexture == nullptr) { mLeftTexture = texture; }
	if(mFrontTexture == nullptr) { mFrontTexture = texture; }
	if(mBackTexture == nullptr) { mBackTexture = texture; }
	if(mUpTexture == nullptr) { mUpTexture = texture; }
	if(mDownTexture == nullptr) { mDownTexture = texture; }
}
