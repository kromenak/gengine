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
    Shader* GetShader() const { return mShader; }
    
    void SetColor(const std::string& name, const Color32& color);
    const Color32* GetColor(const std::string& name) const;
    
    void SetTexture(const std::string& name, Texture* texture);
    Texture* GetTexture(const std::string& name) const;

    void SetFloat(const std::string& name, float value);
    void SetVector4(const std::string& name, const Vector4& vector);
    
    // Helpers for setting/getting frequently accessed shader uniforms.
    void SetColor(const Color32& color) { SetColor("uColor", color); }
    
    void SetDiffuseTexture(Texture* texture) { SetTexture("uDiffuse", texture); }
    Texture* GetDiffuseTexture() const { return GetTexture("uDiffuse"); }

    void SetTranslucent(bool translucent) { mTranslucent = translucent; }
    bool IsTranslucent() const { return mTranslucent; }
	
private:
	static Matrix4 sCurrentViewMatrix;
	static Matrix4 sCurrentProjMatrix;
	static float sAlphaTestValue;
	
    // Shader to use.
    Shader* mShader = nullptr;

    // Material properties/attributes. Maps a uniform variable name to a value.
    // These will be passed to the vertex/fragment shaders during rendering.
    std::unordered_map<std::string, Color32> mColors;
    std::unordered_map<std::string, Texture*> mTextures;
    std::unordered_map<std::string, Vector4> mVectors;
    std::unordered_map<std::string, float> mFloats;

    // If true, this material renders as translucent.
    bool mTranslucent = false;
};
