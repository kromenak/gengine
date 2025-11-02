//
// Clark Kromenaker
//
// Manages loading and caching of assets.
//
#pragma once
#include <initializer_list>
#include <string>
#include <vector>

#include "Asset.h"
#include "AssetCache.h"
#include "BarnFile.h"
#include "StringUtil.h"

class Shader;
class TextAsset;
class Texture;

class AssetManager
{
public:
    void Init();
    void Shutdown();

    // Loose Files
    // Adds a filesystem path to search for assets and bundles at.
    void AddSearchPath(const std::string& searchPath);

    // Given a filename, finds the path to the file if it exists on one of the search paths.
    // Returns empty string if file is not found.
    std::string GetAssetPath(const std::string& fileName) const;
    std::string GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions) const;

    // Barn Files
    // Load or unload a barn bundle.
    bool LoadBarn(const std::string& barnName, BarnSearchPriority priority = BarnSearchPriority::Normal);
    void UnloadBarn(const std::string& barnName);

    // Write an asset from a bundle to a file.
    void WriteBarnAssetToFile(const std::string& assetName);
    void WriteBarnAssetToFile(const std::string& assetName, const std::string& outputDir);

    // Write all assets from a bundle that match a search string.
    void WriteAllBarnAssetsToFile(const std::string& search);
    void WriteAllBarnAssetsToFile(const std::string& search, const std::string& outputDir);

    // Loading (or Getting) Assets
    Texture* LoadSceneTexture(const std::string& name, AssetScope scope = AssetScope::Global);
    TextAsset* LoadLocalizedText(const std::string& name, AssetScope scope = AssetScope::Global);

    Shader* GetShader(const std::string& id);
    Shader* LoadShader(const std::string& idToUse, const std::string& vertexShaderFileNameNoExt, const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags);
    Shader* LoadShader(const std::string& idToUse, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags);

    // Asset Extensions
    template<typename T> void SetExpectedExtension(const std::string& extension, const std::string& assetCacheId = "");

    // Unloading Assets
    void UnloadAssets(AssetScope scope);

    // Querying Assets
    template<typename T> T* LoadAsset(const std::string& name, AssetScope scope = AssetScope::Global, const std::string& assetCacheId = "");
    template<typename T> const std::string_map_ci<T*>* GetAssets(const std::string& assetCacheId = "");

private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    std::vector<std::string> mSearchPaths;

    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    std::string_map_ci<BarnFile> mLoadedBarns;

    // Tracks the highest priority we've seen for a loaded Barn.
    // This just helps us be a bit more efficient (e.g. don't bother searching High priority if no high priority Barns even exist).
    BarnSearchPriority mHighestBarnSearchPriority = BarnSearchPriority::Low;

    // Asset names are often provided without extensions. But an extension is always needed to load from disk.
    // This map lets us automatically set an extension if one is not provided.
    std::unordered_map<TypeId, std::string_map_ci<std::string>> mExpectedAssetExtensionsByType;

    // Retrieve a barn bundle by name, or by contained asset.
    BarnFile* GetBarn(const std::string& barnName);
    BarnFile* GetBarnContainingAsset(const std::string& assetName);

    std::string SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension);

    template<typename T> T* LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache);
    uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize);
};

extern AssetManager gAssetManager;

template<typename T>
void AssetManager::SetExpectedExtension(const std::string& extension, const std::string& assetCacheId)
{
    // The extension can not be empty.
    if(extension.empty()) { return; }

    // Make sure the extension includes the leading period.
    if(extension.front() == '.')
    {
        mExpectedAssetExtensionsByType[T::StaticTypeId()][assetCacheId] = extension;
    }
    else
    {
        mExpectedAssetExtensionsByType[T::StaticTypeId()][assetCacheId] = "." + extension;
    }
}

template<typename T>
T* AssetManager::LoadAsset(const std::string& name, AssetScope scope, const std::string& assetCacheId)
{
    // Get asset cache for this asset type and provided cache ID.
    AssetCache<T>* assetCache = nullptr;
    if(scope != AssetScope::Manual)
    {
        assetCache = AssetCache<T>::Get(assetCacheId);
    }

    // Get the expected extension, if any, for this asset type and provided cache ID.
    std::string expectedExtension = mExpectedAssetExtensionsByType[T::StaticTypeId()][assetCacheId];

    // Either load asset with expected extension added, or just use name passed in.
    if(!expectedExtension.empty())
    {
        return LoadAsset<T>(SanitizeAssetName(name, expectedExtension), scope, assetCache);
    }
    return LoadAsset<T>(name, scope, assetCache);
}

template<typename T>
const std::string_map_ci<T*>* AssetManager::GetAssets(const std::string& assetCacheId)
{
    return &AssetCache<T>::Get(assetCacheId)->GetAssets();
}

template<typename T>
T* AssetManager::LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache)
{
    // If already present in cache, return existing asset right away.
    if(cache != nullptr && scope != AssetScope::Manual)
    {
        T* cachedAsset = cache->GetAsset(name);
        if(cachedAsset != nullptr)
        {
            // One caveat: if the cached asset has a narrower scope than what's being requested, we must PROMOTE the scope.
            // For example, a cached asset with SCENE scope being requested at GLOBAL scope must convert to GLOBAL scope.
            if(cachedAsset->GetScope() == AssetScope::Scene && scope == AssetScope::Global)
            {
                cachedAsset->SetScope(AssetScope::Global);
            }
            return cachedAsset;
        }
    }
    //printf("Loading asset %s\n", assetName.c_str());

    // Create buffer containing this asset's data. If this fails, the asset doesn't exist, so we can't load it.
    AssetData assetData;
    assetData.bytes.reset(CreateAssetBuffer(name, assetData.length));
    if(assetData.bytes == nullptr) { return nullptr; }

    // Create asset from asset buffer.
    std::string upperName = StringUtil::ToUpperCopy(name);
    T* asset = new T(upperName, scope);

    // Add entry in cache, if we have a cache.
    if(asset != nullptr && cache != nullptr && scope != AssetScope::Manual)
    {
        cache->SetAsset(name, asset);
    }

    // Load the asset.
    asset->Load(assetData);
    return asset;
}