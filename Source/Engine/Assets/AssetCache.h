//
// Clark Kromenaker
//
// An asset cache keeps track of which assets have already been loaded, so we avoid loading them multiple times.
// The cache is also useful for iterating all assets to perform some operation (e.g. unload based on a conditon).
// It also supports debug/optimization efforts by providing a way to see all assets of a type that are loaded.
//
#pragma once
#include <mutex>
#include <unordered_map>

#include "Asset.h"      // AssetScope
#include "StringUtil.h" // string_map_ci
#include "TypeId.h"

// This base class is needed to overcome some limitations of a generic type.
class AssetCacheBase
{
public:
    // Tracks all known asset caches in existence, keyed by asset type.
    static std::unordered_map<TypeId, AssetCacheBase*> sAssetCachesByType;

    static AssetCacheBase* GetAssetCache(TypeId typeId);
    static AssetCacheBase* GetAssetCache(TypeId typeId, const std::string& extension);

    virtual ~AssetCacheBase() = default;

protected:
    // An identifier for this asset cache.
    // Useful when multiple caches store the same asset type, but for different purposes.
    std::string mId;

    // A single type is allowed to have multiple asset caches.
    // When multiple asset caches are registered for the same type, a linked list is formed so they can be easily iterated.
    AssetCacheBase* next = nullptr;

    // Can't create this class directly - must use AssetCache<T>.
    AssetCacheBase() = default;
    AssetCacheBase(const std::string& id) : mId(id) { }
};

template<typename T>
class AssetCache : public AssetCacheBase
{
public:
    // The cache itself, keyed by asset name.
    std::string_map_ci<T*> cache;

    AssetCache(const std::string& id = "") : AssetCacheBase(id)
    {
        //TODO: I'd love to move the "Init" code in here, but we have an order-of-initialization problem.
        //TODO: Since AssetManager is global, it may initialize before all class types have initialized. Bummer.
        //TODO: Fixable, but requires some large refactors.
    }

    void Init()
    {
        // Either add the first of this cache, or add to existing one's "next" link.
        auto it = sAssetCachesByType.find(T::StaticTypeId());
        if(it == sAssetCachesByType.end())
        {
            sAssetCachesByType[T::StaticTypeId()] = this;
        }
        else
        {
            next = it->second;
            it->second = this;
        }
    }

    T* Get(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = cache.find(name);
        return it != cache.end() ? it->second : nullptr;
    }

    void Set(const std::string& name, T* asset)
    {
        std::lock_guard<std::mutex> lock(mutex);
        cache[name] = asset;
    }

    void Unload(AssetScope scope = AssetScope::Global)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if(scope == AssetScope::Global)
        {
            // When unloading at global scope, we're really deleting everything and clearing the entire cache.
            for(auto& entry : cache)
            {
                delete entry.second;
            }
            cache.clear();
        }
        else
        {
            // Otherwise, we are picking and choosing what we want to get rid of.
            for(auto it = cache.begin(); it != cache.end();)
            {
                if((*it).second->GetScope() == scope)
                {
                    delete (*it).second;
                    it = cache.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

private:
    // A mutex is required when modifying the cache, since we allow loading assets on any thread.
    // We don't want multiple threads modifying the cache at the same time.
    std::mutex mutex;
};