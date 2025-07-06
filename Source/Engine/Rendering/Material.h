//
// Clark Kromenaker
//
// The material describes the visual output for rendering.
// It indicates the shader to use and any input parameters for the shader (texture, color, etc).
//
#pragma once
#include <map>
#include <unordered_map>

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
    std::unordered_map<std::string, Vector4> mVectors;
    std::unordered_map<std::string, float> mFloats;

    //TODO/HACK: This is a map instead of an unordered map to fix a bug with blob shadows on Mac.
    // On Mac, the textures used for blob shadow get mixed up: uDiffuse goes to texture unit 1 and uLightmap goes to texture unit 0.
    // The reason they get mixed up is not a bug: unordered maps don't guarantee an order.
    // This shouldn't really be a problem - it should still work - but it doesn't! Swapping the texture units causes a bunch of graphical glitches.
    // An easy HACK fix is to use a map instead, since uDiffuse just so happens to sort before uLightmap.
    // BUT...it would be good to get to the root of WHY swapping texture units breaks this...because it shouldn't!
    std::map<std::string, Texture*> mTextures;

    // If true, this material renders as translucent.
    bool mTranslucent = false;
};
