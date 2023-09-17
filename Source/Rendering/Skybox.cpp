#include "Skybox.h"

#include "AssetManager.h"
#include "GAPI.h"
#include "Mesh.h"
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
        if(mFrontTexture != nullptr)
        {
            params.front.width = mFrontTexture->GetWidth();
            params.front.height = mFrontTexture->GetHeight();
            params.front.pixels = mFrontTexture->GetPixelData();
        }
        if(mBackTexture != nullptr)
        {
            params.back.width = mBackTexture->GetWidth();
            params.back.height = mBackTexture->GetHeight();
            params.back.pixels = mBackTexture->GetPixelData();
        }
        if(mUpTexture != nullptr)
        {
            params.top.width = mUpTexture->GetWidth();
            params.top.height = mUpTexture->GetHeight();
            params.top.pixels = mUpTexture->GetPixelData();
        }
        if(mDownTexture != nullptr)
        {
            params.bottom.width = mDownTexture->GetWidth();
            params.bottom.height = mDownTexture->GetHeight();
            params.bottom.pixels = mDownTexture->GetPixelData();
        }
        mCubemapHandle = GAPI::Get()->CreateCubemap(params);
    }
	
	// Activate the material (or fail).
	mMaterial.Activate(Matrix4::Identity);
	
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
