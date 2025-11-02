#include "Shader.h"

#include "AssetManager.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "TextAsset.h"

TYPEINFO_INIT(Shader, Asset, GENERATE_TYPE_ID)
{

}

Shader::Shader(const std::string& name, const std::string& vertexShaderFileNameNoExt, const std::string& fragmentShaderFileNameNoExt,
               const std::vector<std::string>& featureFlags) : Asset(name, AssetScope::Manual)
{
    // Load text assets for vertex and fragment shader sources.
    std::string vertexShaderFileName = Path::SetExtension(vertexShaderFileNameNoExt, GAPI::Get()->GetShaderFileExtension());
    TextAsset* vertexShaderSource = gAssetManager.LoadAsset<TextAsset>(vertexShaderFileName, AssetScope::Manual, "shader_source");

    std::string fragmentShaderFileName = Path::SetExtension(fragmentShaderFileNameNoExt, GAPI::Get()->GetShaderFileExtension());
    TextAsset* fragmentShaderSource = gAssetManager.LoadAsset<TextAsset>(fragmentShaderFileName, AssetScope::Manual, "shader_source");

    // Create from source if both source files loaded successfully.
    if(vertexShaderSource != nullptr && fragmentShaderSource != nullptr)
    {
        CreateShader(vertexShaderSource, fragmentShaderSource, featureFlags);
    }

    // Delete text assets after use.
    delete vertexShaderSource;
    delete fragmentShaderSource;
}

Shader::Shader(const std::string& name, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags) : Asset(name, AssetScope::Manual)
{
    // Load text asset containing shader source.
    std::string shaderFileNameWithExt = Path::SetExtension(shaderFileNameNoExt, GAPI::Get()->GetShaderFileExtension());
    TextAsset* shaderSource = gAssetManager.LoadAsset<TextAsset>(shaderFileNameWithExt, AssetScope::Manual, "shader_source");

    // Create shader from source.
    // When a single source file is provided, we assume both vertex and fragment shader are in one file,
    // wrapped in #defines for VERTEX_SHADER and FRAGMENT_SHADER.
    if(shaderSource != nullptr)
    {
        CreateShader(shaderSource, shaderSource, featureFlags);
    }

    // Delete text assets after use.
    delete shaderSource;
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

void Shader::CreateShader(TextAsset* vertexShaderText, TextAsset* fragmentShaderText, const std::vector<std::string>& featureFlags)
{
    GAPI::ShaderParams shaderParams;
    shaderParams.vertexShaderSource = reinterpret_cast<char*>(vertexShaderText->GetText());
    shaderParams.fragmentShaderSource = reinterpret_cast<char*>(fragmentShaderText->GetText());
    shaderParams.featureFlags = featureFlags;
    mShaderHandle = GAPI::Get()->CreateShader(shaderParams);
}