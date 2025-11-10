//
// Clark Kromenaker
//
// Acts as a central hub for loading, caching, and managing assets.
// Provides the following key features:
//
// 1) Ordered search paths: provide a list of paths at which to search for loose file assets or asset archives.
//    Assets or asset archives are loaded at the first path they are discovered at.
//
// 2) Loose file path resolution: provide a file name, its full path will be resolved to one of the search paths (if it exists).
//    Multiple potential file extensions can also be provided and checked.
//
// 3) Loading of asset archives: rather than only using loose files, assets can be bundled into archives for distribution.
//    Multiple different types of asset archives can be implemented.
//
// 4) Extracting assets from archives: if an asset exists in a loaded archive, it can be extracted to the disk by name.
//
// 5) Load assets to C++ class representation and cache for later retrieval.
//    When an asset is loaded, it is stored in an Asset Cache. Subsequent retrievals return the cached instance.
//
// 6) When loading assets, you can specify the full name with extension, or just the name.
//    If only the name is provided, an "asset name resolver" can be provided that maps asset types and cache IDs to expected extensions.
//    The system will then try to use the expected extensions to find and load the correct asset.
//
// 7) Asset unloading via scope: each asset stores a scope (Global, Scene, etc). Assets can be unloaded by scope at any time.
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
    // Paths to search when loading loose files (both individual assets and archives).
    void AddSearchPath(const std::string& searchPath);
    void RemoveSearchPath(const std::string& searchPath);

    // Loose File Paths
    // Finds the full path of a loose file (either an individual asset or an archive). Returns empty string if not found.
    std::string FindLooseFilePath(const std::string& fileName) const;
    std::string FindLooseFilePath(const std::string& fileName, std::initializer_list<std::string> extensions) const;

    // Asset Archives
    bool LoadAssetArchive(const std::string& archiveName, int searchOrder = 0);

    // Asset Extraction
    void SetAssetExtractor(const std::string& extension, const std::function<bool(AssetExtractData&)>& extractorFunction);
    bool ExtractAsset(const std::string& assetName, const std::string& outputDirectory = "") const;
    void ExtractAssets(const std::string& search, const std::string& outputDirectory = "");

    // Asset Loading/Unloading
    void SetAssetNameResolver(const AssetNameResolver& resolver) { mAssetNameResolver = resolver; }
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
