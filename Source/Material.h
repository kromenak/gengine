//
// Material.h
//
// Clark Kromenaker
//
// The material describes the visual output for rendering.
// It indicates the shader to use and any input parameters for the shader.
//
#pragma once
#include "Color32.h"

class Shader;
class Texture;
class Matrix4;

class Material
{
public:
    static Shader* sDefaultShader;
	
	
    Material();
    
    bool operator==(const Material& other) const;
	
    void SetShader(Shader* shader) { mShader = shader; }
	
	void SetColor(const Color32& color) { mColor = color; }
    void SetDiffuseTexture(Texture* diffuseTexture) { mDiffuseTexture = diffuseTexture; }
	
	void Activate();
	
	void SetWorldTransformMatrix(const Matrix4& matrix);
	void SetActiveColor(const Color32& color);
	
private:
    // Shader to use.
    //TODO: Could have array of shaders to represent "subshaders" for different hardware.
    Shader* mShader = nullptr;
	
	// A uniform color to pass to the shader.
	Color32 mColor = Color32::White;
	
    // Diffuse texture.
    Texture* mDiffuseTexture = nullptr;
    
    //TODO: Opaque vs. transparent? Render queue value?
};
