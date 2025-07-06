//
// Clark Kromenaker
//
// A compiled and linked shader program.
//
#pragma once
#include "Asset.h"

#include <string>

class Color32;
class Matrix4;
class TextAsset;
class Vector3;
class Vector4;

enum class UniformType
{
    Unknown,

    Float,
    Int,
    Uint,
    Bool,

    Vector2,
    Vector3,
    Vector4,

    Matrix2,
    Matrix3,
    Matrix4,

    Texture2D,
    TextureCube
    //TODO: Add more as needed
};

struct Uniform
{
    // Type of the uniform.
    UniformType type = UniformType::Unknown;

    // Uniform name.
    std::string name;
};

class Shader : public Asset
{
    TYPEINFO_SUB(Shader, Asset);
public:
    Shader(const std::string& name, TextAsset* vertShaderBytes, TextAsset* fragShaderBytes);
    ~Shader();

    void Activate();

    void SetUniformInt(const char* name, int value);
    void SetUniformFloat(const char* name, float value);

    void SetUniformVector3(const char* name, const Vector3& vector);
    void SetUniformVector4(const char* name, const Vector4& vector);

    void SetUniformMatrix4(const char* name, const Matrix4& mat);

    void SetUniformColor(const char* name, const Color32& color);

    bool IsGood() const { return mShaderHandle != nullptr; }

private:
    // Handle to shader in underlying graphics system.
    void* mShaderHandle = nullptr;
};
