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

    // Asset Archives
    bool LoadAssetArchive(const std::string& archiveName, int searchOrder = 0);

    // Asset Extraction
    void SetAssetExtractor(const std::string& extension, const std::function<bool(AssetExtractData&)>& extractorFunction);
    bool ExtractAsset(const std::string& assetName, const std::string& outputDirectory = "") const;
    void ExtractAssets(const std::string& search, const std::string& outputDirectory = "");

    // Asset Extensions
    template<typename T> void SetExpectedExtension(const std::string& extension, const std::string& assetCacheId = "");

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

    // Asset names are often provided without extensions. But an extension is always needed to load from disk.
    // This map lets us automatically set an extension if one is not provided.
    std::unordered_map<TypeId, std::string_map_ci<std::string>> mExpectedAssetExtensionsByType;

    // Maps an asset extension to a custom extractor function.
    // Many assets can simply be written to disk byte-for-byte. But some can require custom processing.
    std::unordered_map<std::string, std::function<bool(AssetExtractData&)>> mAssetExtractorsByExtension;

    bool ExtractAsset(IAssetArchive* archive, const std::string& assetName, const std::string& outputDirectory) const;
    std::string SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension);
    uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize) const;
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

template<typename T>
const std::string_map_ci<T*>& AssetManager::GetAssets(const std::string& assetCacheId)
{
    return AssetCache<T>::Get(assetCacheId)->GetAssets();
}