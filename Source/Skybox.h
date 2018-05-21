//
// Skybox.h
//
// Clark Kromenaker
//
// Data and routines used to display a skybox around the world.
//
#pragma once
#include <GL/glew.h>

class Texture;
class GLVertexArray;

class Skybox
{
public:
    void Render();
    
    void SetRightTexture(Texture* texture) { mRightTexture = texture; }
    void SetLeftTexture(Texture* texture) { mLeftTexture = texture; }
    void SetFrontTexture(Texture* texture) { mFrontTexture = texture; }
    void SetBackTexture(Texture* texture) { mBackTexture = texture; }
    void SetUpTexture(Texture* texture) { mUpTexture = texture; }
    void SetDownTexture(Texture* texture) { mDownTexture = texture; }
    
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
    GLVertexArray* mVertexArray = nullptr;
    
    // An ID for the texture object generated in OpenGL.
    GLuint mCubemapTextureId = GL_NONE;
};
