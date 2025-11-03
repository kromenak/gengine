//
// Clark Kromenaker
//
//
//
#pragma once
#include <string>
#include <vector>

class Shader;

namespace ShaderCache
{
    Shader* GetShader(const std::string& id);
    Shader* LoadShader(const std::string& idToUse, const std::string& vertexShaderFileNameNoExt, const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags);
    Shader* LoadShader(const std::string& idToUse, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags);
}