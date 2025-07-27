//
// Clark Kromenaker
//
// A compiled and linked shader program.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>

class Color32;
class Matrix4;
class TextAsset;
class Vector3;
class Vector4;

class Shader : public Asset
{
    TYPEINFO_SUB(Shader, Asset);
public:
    Shader(const std::string& name, const std::string& vertexShaderFileNameNoExt, const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags);
    Shader(const std::string& name, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags);
    ~Shader();

    void Activate();

    void SetUniformInt(const char* name, int value);
    void SetUniformFloat(const char* name, float value);

    void SetUniformVector3(const char* name, const Vector3& vector);
    void SetUniformVector4(const char* name, const Vector4& vector);

    void SetUniformMatrix4(const char* name, const Matrix4& mat);

    void SetUniformColor(const char* name, const Color32& color);

    bool IsValid() const { return mShaderHandle != nullptr; }

private:
    // Handle to shader in underlying graphics system.
    void* mShaderHandle = nullptr;

    void CreateShader(TextAsset* vertexShaderText, TextAsset* fragmentShaderText, const std::vector<std::string>& featureFlags);
};
