//
// Clark Kromenaker
//
// Data and routines used to display a skybox around the world.
//
#pragma once
#include "GAPI.h"
#include "Material.h"
#include "Matrix4.h"

class Mesh;
class Ray;
struct RaycastHit;
class Texture;

class Skybox
{
public:
    ~Skybox();

    void Render();
    
	void SetRightTexture(Texture* texture);
	void SetLeftTexture(Texture* texture);
	void SetFrontTexture(Texture* texture);
	void SetBackTexture(Texture* texture);
	void SetUpTexture(Texture* texture);
	void SetDownTexture(Texture* texture);

    void LoadMaskTextures();

    void SetRightMaskTexture(Texture* texture) { mRightMaskTexture = texture; }
    void SetLeftMaskTexture(Texture* texture) { mLeftMaskTexture = texture; }
    void SetFrontMaskTexture(Texture* texture) { mFrontMaskTexture = texture; }
    void SetBackMaskTexture(Texture* texture) { mBackMaskTexture = texture; }
    void SetUpMaskTexture(Texture* texture) { mUpMaskTexture = texture; }
    void SetDownMaskTexture(Texture* texture) { mDownMaskTexture = texture; }
    
    void SetAzimuth(float radians);

    uint8_t Raycast(const Ray& ray);

private:
    // Textures for various sides of the skybox.
    Texture* mRightTexture = nullptr;
    Texture* mLeftTexture = nullptr;
    Texture* mFrontTexture = nullptr;
    Texture* mBackTexture = nullptr;
    Texture* mUpTexture = nullptr;
    Texture* mDownTexture = nullptr;

    // Each side of the skybox can potentially have a mask texture associated with it.
    // The mask is used to identify regions of the skybox for raycasting purposes.
    Texture* mRightMaskTexture = nullptr;
    Texture* mLeftMaskTexture = nullptr;
    Texture* mFrontMaskTexture = nullptr;
    Texture* mBackMaskTexture = nullptr;
    Texture* mUpMaskTexture = nullptr;
    Texture* mDownMaskTexture = nullptr;
    
    // Vertex array for the big cube we will render the skybox onto.
    Mesh* mSkyboxMesh = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    TextureHandle mCubemapHandle = nullptr;

    // In GK3, skyboxes can be rotated (via the azimuth keyword).
    // This is occasionally used to put landmarks on the skybox in correct spots in the sky.
    Matrix4 mRotationMatrix;
	
	// Material used for rendering.
	Material mMaterial;
	
	void SetDefaultTexture(Texture* texture);
};
