#include "ShaderCache.h"

#include "AssetCache.h"
#include "Shader.h"

Shader* ShaderCache::GetShader(const std::string& id)
{
    // Attempt to return a cached shader.
    return AssetCache<Shader>::Get()->GetAsset(id);
}

Shader* ShaderCache::LoadShader(const std::string& idToUse, const std::string& vertexShaderFileNameNoExt,
                                 const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags)
{
    // If shader with this ID is already loaded, return that.
    Shader* cachedShader = GetShader(idToUse);
    if(cachedShader != nullptr)
    {
        return cachedShader;
    }

    // Otherwise, create a new shader.
    Shader* shader = new Shader(idToUse, vertexShaderFileNameNoExt, fragmentShaderFileNameNoExt, featureFlags);

    // If not valid, delete and return null. Log should display compiler error.
    if(!shader->IsValid())
    {
        delete shader;
        return nullptr;
    }

    // Cache and return.
    AssetCache<Shader>::Get()->SetAsset(idToUse, shader);
    return shader;
}

Shader* ShaderCache::LoadShader(const std::string& idToUse, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags)
{
    // Very similar to above, but assumes both vertex and fragment shader are stored in a single source file.
    // Return cached shader if one exists.
    Shader* cachedShader = GetShader(idToUse);
    if(cachedShader != nullptr)
    {
        return cachedShader;
    }

    // Create new shader.
    Shader* shader = new Shader(idToUse, shaderFileNameNoExt, featureFlags);

    // If not valid, delete and return null. Log should display compiler error.
    if(!shader->IsValid())
    {
        delete shader;
        return nullptr;
    }

    // Cache and return.
    AssetCache<Shader>::Get()->SetAsset(idToUse, shader);
    return shader;
}