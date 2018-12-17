//
// Skybox.cpp
//
// Clark Kromenaker
//
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

Skybox::Skybox()
{
	
}

Skybox::~Skybox()
{
	
}

void Skybox::Render()
{
    // Generate submesh on the fly, if not yet generated.
    if(mSkyboxMesh == nullptr)
    {
        mSkyboxMesh = new Mesh(36, 3 * sizeof(float), MeshUsage::Static);
        mSkyboxMesh->SetPositions(points);
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
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapTextureId);
    
    mSkyboxMesh->Render();
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
