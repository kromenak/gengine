//
// Clark Kromenaker
//
// An asset cache tracks assets already loaded into memory. We can reuse them instead of loading multiple copies.
// It also provides a list of loaded assets by type, which can be useful for profiling, optimizing, and debugging.
//
#pragma once
#include <mutex>
#include <unordered_map>
#include <vector>

#include "Asset.h"      // AssetScope
#include "StringUtil.h" // string_map_ci
#include "TypeId.h"

// Problem: we want to track all the asset caches that exist in a static list, but templatized classes can't be put in a list (for different types of T).
// To get around this, we can have a base class (essentially just an interface) that CAN be stored in a list in a homogenous fashion.

// We want to track a map of all known asset caches. But since AssetCache is templated, we can't!
// To get around this, we utilize an empty base class (essentially an interface) that CAN be put in a map as a pointer.
class IAssetCache
{
public:
    // Tracks all known asset caches in existence, keyed by asset type.
    static std::unordered_map<TypeId, std::vector<IAssetCache*>> sAssetCachesByType;
    static std::mutex sAssetCachesMutex;

    virtual ~IAssetCache() = default;
    virtual const std::string& GetId() = 0;
    virtual void UnloadAssets(AssetScope scope) = 0;
};

template<typename T>
class AssetCache : public IAssetCache
{
public:
    static AssetCache<T>* Get(const std::string& id = "")
    {
        // This code could run on multiple threads, so we should guard reads/writes to the static collection.
        std::lock_guard<std::mutex> lock(sAssetCachesMutex);

        // Get all asset caches that exist for this type.
        // This creates a new list if the type is not yet in the map.
        std::vector<IAssetCache*>& assetCachesForType = sAssetCachesByType[T::StaticTypeId()];

        // If none yet exist, ensure that the first one is always a default cache with an empty ID.
        if(assetCachesForType.empty())
        {
            assetCachesForType.push_back(new AssetCache<T>());
        }

        // Find the asset cache that matches the passed in ID.
        for(IAssetCache* assetCache : assetCachesForType)
        {
            if(StringUtil::EqualsIgnoreCase(assetCache->GetId(), id))
            {
                return static_cast<AssetCache<T>*>(assetCache);
            }
        }

        // If none matches, we should create a new one with the desired ID.
        AssetCache<T>* newCache = new AssetCache<T>(id);
        assetCachesForType.push_back(newCache);
        return newCache;
    }

    explicit AssetCache(const std::string& id = "") :
        mId(id)
    {

    }

    const std::string& GetId() override { return mId; }

    T* GetAsset(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mAssetsMutex);
        auto it = mAssets.find(name);
        return it != mAssets.end() ? it->second : nullptr;
    }

    void SetAsset(const std::string& name, T* asset)
    {
        std::lock_guard<std::mutex> lock(mAssetsMutex);
        mAssets[name] = asset;
    }

    void UnloadAssets(AssetScope scope) override
    {
        std::lock_guard<std::mutex> lock(mAssetsMutex);
        if(scope == AssetScope::Global)
        {
            // When unloading at global scope, we're really deleting everything and clearing the entire cache.
            for(auto& entry : mAssets)
            {
                delete entry.second;
            }
            mAssets.clear();
        }
        else
        {
            // Otherwise, we are picking and choosing what we want to get rid of.
            for(auto it = mAssets.begin(); it != mAssets.end();)
            {
                if((*it).second->GetScope() == scope)
                {
                    delete (*it).second;
                    it = mAssets.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    const std::string_map_ci<T*>& GetAssets() const { return mAssets; }

private:
    // An identifier for this asset cache.
    // Useful when multiple caches store the same asset type, but for different purposes.
    std::string mId;

    // The assets themselves, keyed by name.
    std::string_map_ci<T*> mAssets;

    // A mutex is required when modifying the cache, since we allow loading assets on any thread.
    // We don't want multiple threads modifying the cache at the same time.
    std::mutex mAssetsMutex;
};