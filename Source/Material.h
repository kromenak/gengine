//
// Material.h
//
// Clark Kromenaker
//
// The material describes the visual output for rendering.
// It indicates the shader to use and any input parameters for the shader (texture, color, etc).
//
#pragma once
#include <unordered_map>
#include <vector>

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
    
	void Activate(const Matrix4& objectToWorldMatrix);
	
    void SetShader(Shader* shader) { mShader = shader; }
    
    void SetColor(const std::string& name, const Color32& color);
    //GetColor
    
    void SetTexture(const std::string& name, Texture* texture);
    Texture* GetTexture(const std::string& name) const;
    
    // Helpers for setting/getting frequently accessed shader uniforms.
    void SetColor(const Color32& color) { SetColor("uColor", color); }
    
    void SetDiffuseTexture(Texture* texture) { SetTexture("uDiffuse", texture); }
    Texture* GetDiffuseTexture() const { return GetTexture("uDiffuse"); }
    
	bool IsTranslucent();
	
private:
	static Matrix4 sCurrentViewMatrix;
	static Matrix4 sCurrentProjMatrix;
	static float sAlphaTestValue;
	
    // Shader to use.
    Shader* mShader = nullptr;
    
    std::unordered_map<std::string, Color32> mColors;
    std::unordered_map<std::string, Texture*> mTextures;
    
    //TODO: Opaque vs. transparent? Render queue value?
};
