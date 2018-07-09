//
// Material.h
//
// Clark Kromenaker
//
// The material describes the visual output for rendering.
// It indicates the shader to use and any input parameters for the shader.
//
#pragma once

class Shader;
class Texture;
class Matrix4;

class Material
{
public:
    static Shader* sDefaultShader;
    
    Material();
    
    bool operator==(const Material& other) const;
    
    void Activate();
    
    void SetShader(Shader* shader) { mShader = shader; }
    
    void SetWorldTransformMatrix(const Matrix4& mat);
    
    void SetDiffuseTexture(Texture* diffuseTexture) { mDiffuseTexture = diffuseTexture; }
    
private:
    // Shader to use.
    //TODO: Could have array of shaders to represent "subshaders" for different hardware.
    Shader* mShader = nullptr;
    
    // Diffuse texture.
    Texture* mDiffuseTexture = nullptr;
    
    //TODO: Opaque vs. transparent? Render queue value?
};
