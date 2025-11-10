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
#include "AssetNameResolver.h"
#include "IAssetArchive.h"
#include "StringUtil.h"

// Helper struct used when extracting an asset.
struct AssetExtractData
{
    // The name of the asset being extracted.
    std::string assetName;

    // The byte data of the asset to be extracted.
    AssetData assetData;

    // The path to extract the asset to.
    std::string outputPath;
};

class AssetManager
{
public:
    void Shutdown();

    // Search Paths
    void AddSearchPath(const std::string& searchPath);
    void RemoveSearchPath(const std::string& searchPath);

    // Asset Paths
    // Finds the full path to an asset on any search path, or empty string if asset can't be found.
    std::string GetAssetPath(const std::string& fileName) const;
    std::string GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions) const;

    void SetAssetNameResolver(const AssetNameResolver& resolver) { mAssetNameResolver = resolver; }

    // Asset Archives
    bool LoadAssetArchive(const std::string& archiveName, int searchOrder = 0);

    // Asset Extraction
    void SetAssetExtractor(const std::string& extension, const std::function<bool(AssetExtractData&)>& extractorFunction);
    bool ExtractAsset(const std::string& assetName, const std::string& outputDirectory = "") const;
    void ExtractAssets(const std::string& search, const std::string& outputDirectory = "");

    // Asset Loading/Unloading
    template<typename T> T* LoadAsset(const std::string& name, AssetScope scope = AssetScope::Global, const std::string& assetCacheId = "");
    template<typename T> T* LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache);
    template<typename T> const std::string_map_ci<T*>& GetAssets(const std::string& assetCacheId = "");
    void UnloadAssets(AssetScope scope);

private:
    // Search paths for loading assets from the disk. Used for loading loose files and asset archives.
    // Expected to be in priority order - an asset is loaded from the first place it is found.
    std::vector<std::string> mSearchPaths;

    // A set of asset archives that have been loaded. Each archive can contain many assets to be loaded.
    // Again, in priority order - an asset is loaded from the first archive it is found in.
    struct AssetArchive
    {
        int searchOrder = 0;
        IAssetArchive* archive = nullptr;
    };
    std::vector<AssetArchive> mArchives;

    // Used to determine whether asset names have valid extensions, and to map certain asset types to particular extensions.
    // This is mostly important because assets are often provided without extensions - we need to figure out the full asset name to load from disk or archive!
    AssetNameResolver mAssetNameResolver;

    // Maps an asset extension to a custom extractor function.
    // Many assets can simply be written to disk byte-for-byte. But some can require custom processing.
    std::unordered_map<std::string, std::function<bool(AssetExtractData&)>> mAssetExtractorsByExtension;

    bool ExtractAsset(IAssetArchive* archive, const std::string& assetName, const std::string& outputDirectory) const;
    uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize) const;
    template<typename T> T* LoadAssetInternal(const std::string& name, AssetScope scope, AssetCache<T>* cache);
};

extern AssetManager gAssetManager;

template<typename T>
T* AssetManager::LoadAsset(const std::string& name, AssetScope scope, const std::string& assetCacheId)
{
    // Get asset cache for this asset type and provided cache ID.
    AssetCache<T>* assetCache = nullptr;
    if(scope != AssetScope::Manual)
    {
        assetCache = AssetCache<T>::Get(assetCacheId);
    }

    // Pass on to LoadAsset with a cache pointer.
    return LoadAsset(name, scope, assetCache);
}

template<typename T>
T* AssetManager::LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache)
{
    // If the asset name already has a valid extension, assume the caller knows what they're doing.
    // Just load the asset with that name, as-is.
    if(mAssetNameResolver.HasValidExtension(name))
    {
        return LoadAssetInternal<T>(name, scope, cache);
    }
    else
    {
        // The asset name doesn't have an extension. But one is likely needed to load the asset from disk.
        // So we need to guess the extension, based on the type extensions registered in the asset name resolver.
        for(const std::string& extension : mAssetNameResolver.GetTypeExtensions<T>(cache != nullptr ? cache->GetId() : ""))
        {
            // Attempt to load the asset using this extension. If it works, the result will be non-null.
            T* asset = LoadAssetInternal<T>(name + extension, scope, cache);
            if(asset != nullptr)
            {
                return asset;
            }
        }

        // Worst case, this could be an asset with a non-standard extension or no extension at all.
        // Try to load just using the passed in name as-is.
        return LoadAssetInternal<T>(name, scope, cache);
    }
}

template<typename T>
const std::string_map_ci<T*>& AssetManager::GetAssets(const std::string& assetCacheId)
{
    return AssetCache<T>::Get(assetCacheId)->GetAssets();
}

template<typename T>
inline T* AssetManager::LoadAssetInternal(const std::string& name, AssetScope scope, AssetCache<T>* cache)
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

    // Create buffer containing this asset's data. If this fails, the asset doesn't exist, so we can't load it.
    AssetData assetData;
    assetData.bytes.reset(CreateAssetBuffer(name, assetData.length));
    if(assetData.bytes == nullptr) { return nullptr; }
    //printf("Loading asset %s\n", assetName.c_str());

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
