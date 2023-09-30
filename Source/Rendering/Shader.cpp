#include "Shader.h"

#include "GAPI.h"
#include "TextAsset.h"

Shader::Shader(const std::string& name, TextAsset* vertShaderBytes, TextAsset* fragShaderBytes) : Asset(name)
{
    mShaderHandle = GAPI::Get()->CreateShader(vertShaderBytes->GetText(),
                                              fragShaderBytes->GetText());
}

Shader::~Shader()
{
    GAPI::Get()->DestroyShader(mShaderHandle);
}

void Shader::Activate()
{
    GAPI::Get()->ActivateShader(mShaderHandle);
}

void Shader::SetUniformInt(const char* name, int value)
{
    GAPI::Get()->SetShaderUniformInt(mShaderHandle, name, value);
}

void Shader::SetUniformFloat(const char* name, float value)
{
    GAPI::Get()->SetShaderUniformFloat(mShaderHandle, name, value);
}

void Shader::SetUniformVector3(const char* name, const Vector3& vector)
{
    GAPI::Get()->SetShaderUniformVector3(mShaderHandle, name, vector);
}

void Shader::SetUniformVector4(const char *name, const Vector4& vector)
{
    GAPI::Get()->SetShaderUniformVector4(mShaderHandle, name, vector);
}

void Shader::SetUniformMatrix4(const char* name, const Matrix4& mat)
{
    GAPI::Get()->SetShaderUniformMatrix4(mShaderHandle, name, mat);
}

void Shader::SetUniformColor(const char* name, const Color32& color)
{
    GAPI::Get()->SetShaderUniformColor(mShaderHandle, name, color);
}
