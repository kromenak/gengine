//
// Material.h
//
// Clark Kromenaker
//
// The material describes the visual output for rendering.
// It indicates the shader to use and any input parameters for the shader (texture, color, etc).
//
#pragma once
#include "Color32.h"
#include "Matrix4.h"

class Shader;
class Texture;

class Material
{
public:
    static Shader* sDefaultShader;
	static void SetViewMatrix(const Matrix4& viewMatrix);
	static void SetProjMatrix(const Matrix4& projMatrix);
	static void UseAlphaTest(bool use);
	
    Material();
	Material(Shader* shader);
    
    bool operator==(const Material& other) const;
	
	void Activate();
	
    void SetShader(Shader* shader) { mShader = shader; }
    
	void SetColor(const Color32& color) { mColor = color; }
	
	Texture* GetDiffuseTexture() const { return mDiffuseTexture; }
    void SetDiffuseTexture(Texture* diffuseTexture) { mDiffuseTexture = diffuseTexture; }
	
	void SetWorldTransformMatrix(const Matrix4& matrix);
	void SetActiveColor(const Color32& color);
	
	bool IsTranslucent();
	
private:
	static Matrix4 sCurrentViewMatrix;
	static Matrix4 sCurrentProjMatrix;
	static float sAlphaTestValue;
	
    // Shader to use.
    Shader* mShader = nullptr;
	
	// A uniform color to pass to the shader.
	Color32 mColor = Color32::White;
	
    // Diffuse texture.
    Texture* mDiffuseTexture = nullptr;
    
    //TODO: Opaque vs. transparent? Render queue value?
};
