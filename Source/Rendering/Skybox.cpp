#include "Skybox.h"

#include "Mesh.h"
#include "Services.h"
#include "Texture.h"

float points[] = {
    // Back
    -10.0f,  10.0f, -10.0f, // Top-left
    -10.0f, -10.0f, -10.0f, // Bottom-left
     10.0f, -10.0f, -10.0f, // Bottom-right
     10.0f, -10.0f, -10.0f, // Bottom-right
     10.0f,  10.0f, -10.0f, // Top-right
    -10.0f,  10.0f, -10.0f, // Top-left
    
    // Left
    -10.0f, -10.0f,  10.0f,
    -10.0f, -10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f,  10.0f,
    -10.0f, -10.0f,  10.0f,
    
    // Right
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    
    // Front
    -10.0f, -10.0f,  10.0f,
    -10.0f,  10.0f,  10.0f,
     10.0f,  10.0f,  10.0f,
     10.0f,  10.0f,  10.0f,
     10.0f, -10.0f,  10.0f,
    -10.0f, -10.0f,  10.0f,
    
    // Top
    -10.0f,  10.0f, -10.0f, // Back-left
     10.0f,  10.0f, -10.0f, // Back-right
     10.0f,  10.0f,  10.0f, // Front-right
     10.0f,  10.0f,  10.0f, // Front-right
    -10.0f,  10.0f,  10.0f, // Front-left
    -10.0f,  10.0f, -10.0f, // Back-left

    // Bottom
    -10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f,  10.0f,
     10.0f, -10.0f, -10.0f,
     10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f,  10.0f,
     10.0f, -10.0f,  10.0f
};

void Skybox::Render()
{
    // Generate submesh on the fly, if not yet generated.
    if(mSkyboxMesh == nullptr)
    {
        MeshDefinition meshDefinition(MeshUsage::Static, 36);
        meshDefinition.SetVertexLayout(VertexLayout::Packed);
        meshDefinition.AddVertexData(VertexAttribute::Position, points);
        
        mSkyboxMesh = new Mesh();
        mSkyboxMesh->AddSubmesh(meshDefinition);
        
        // Also use this opportunity to set the shader on the material.
        Shader* skyboxShader = Services::GetAssets()->LoadShader("3D-Skybox");
        mMaterial.SetShader(skyboxShader);
    }
    
    if(mCubemapTextureId == GL_NONE)
    {
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &mCubemapTextureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapTextureId);
        
        // Create each texture for the cubemap.
        // Note that we MUST create 6 textures, or the cubemap will not display properly.
        // Also, Front is -Z and Back is +Z. Not sure if this indicates a bug, or a conversion done in the original game, or what?
        if(mRightTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA,
                         mRightTexture->GetWidth(), mRightTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mRightTexture->GetPixelData());
        }
        if(mLeftTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA,
                         mLeftTexture->GetWidth(), mLeftTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mLeftTexture->GetPixelData());
        }
        if(mFrontTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
                         mFrontTexture->GetWidth(), mFrontTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mFrontTexture->GetPixelData());
        }
        if(mBackTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA,
                         mBackTexture->GetWidth(), mBackTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mBackTexture->GetPixelData());
        }
        if(mUpTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA,
                         mUpTexture->GetWidth(), mUpTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mUpTexture->GetPixelData());
        }
        if(mDownTexture != nullptr)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA,
                         mDownTexture->GetWidth(), mDownTexture->GetHeight(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, mDownTexture->GetPixelData());
        }
        
        // These settings help to avoid visible seams around the edges of the skybox.
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
	
	// Activate the material (or fail).
	mMaterial.Activate(Matrix4::Identity);
	
	// Activate and bind the cubemap texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapTextureId);
	
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
