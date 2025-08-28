//
// Clark Kromenaker
//
// A skybox displays up to six textures around the game's 3D world that are seemingly infinitely far away.
//
// This is accomplished internally by rendering a small cube centered on the camera at the start of a frame render.
// Then, the world is rendered on top of the skybox to partially/mostly cover it.
//
#pragma once
#include "GAPI.h"
#include "Material.h"
#include "Matrix4.h"

class Mesh;
class Ray;
class Texture;

// To create a skybox, you must specify textures for each side.
// At least one texture must be provided, and it's recommended to provide all (with down being the least important).
struct SkyboxTextures
{
    // This union approach allows referencing textures by specific name, or via an array.
    union
    {
        struct
        {
            Texture* left;
            Texture* right;
            Texture* back;
            Texture* front;
            Texture* bottom;
            Texture* top;
        } named;
        Texture* array[6] = { 0 };
    };
};

class Skybox
{
public:
    Skybox(const SkyboxTextures& textures);
    ~Skybox();

    void Render();

    void SetAzimuth(float radians) { mRotationMatrix = Matrix4::MakeRotateY(radians); }

    uint8_t Raycast(const Ray& ray);

private:
    // Vertex array for the big cube we will render the skybox onto.
    Mesh* mSkyboxMesh = nullptr;

    // An ID for the texture object generated in OpenGL.
    TextureHandle mCubemapHandle = nullptr;

    // In GK3, skyboxes can be rotated (via the azimuth keyword).
    // This is occasionally used to put landmarks on the skybox in correct spots in the sky.
    Matrix4 mRotationMatrix = Matrix4::Identity;

    // Material used for rendering.
    Material mMaterial;

    // Textures used for the sides of the skybox.
    SkyboxTextures mTextures;

    // Each side of the skybox can optionally have a mask texture associated with it.
    // The mask is used to identify regions of the skybox for raycasting purposes.
    SkyboxTextures mMaskTextures;

    void CreateMeshAndCubemap();
};
