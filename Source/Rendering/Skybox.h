//
// Clark Kromenaker
//
// Data and routines used to display a skybox around the world.
//
#pragma once
#include <GL/glew.h>

#include "Material.h"

class Mesh;
class Texture;

class Skybox
{
public:
    void Render();
    
	void SetRightTexture(Texture* texture);
	void SetLeftTexture(Texture* texture);
	void SetFrontTexture(Texture* texture);
	void SetBackTexture(Texture* texture);
	void SetUpTexture(Texture* texture);
	void SetDownTexture(Texture* texture);
    
    void SetAzimuth(float azimuth) { mAzimuth = azimuth; }
	
private:
    // Textures for various sides of the skybox.
    Texture* mRightTexture = nullptr;
    Texture* mLeftTexture = nullptr;
    Texture* mFrontTexture = nullptr;
    Texture* mBackTexture = nullptr;
    Texture* mUpTexture = nullptr;
    Texture* mDownTexture = nullptr;
    
    // The angle of the sun around the horizon.
    float mAzimuth = 0.0f;
    
    // Vertex array for the big cube we will render the skybox onto.
    Mesh* mSkyboxMesh = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mCubemapTextureId = GL_NONE;
	
	// Material used for rendering.
	Material mMaterial;
	
	void SetDefaultTexture(Texture* texture);
};
