#include "AssetCache.h"

std::unordered_map<TypeId, AssetCacheBase*> AssetCacheBase::sAssetCachesByType;

AssetCacheBase* AssetCacheBase::GetAssetCache(TypeId typeId)
{
    auto it = sAssetCachesByType.find(typeId);
    if(it != sAssetCachesByType.end())
    {
        return it->second;
    }
    return nullptr;
}

AssetCacheBase* AssetCacheBase::GetAssetCache(TypeId typeId, const std::string& extension)
{
    AssetCacheBase* assetCache = GetAssetCache(typeId);
    while(assetCache != nullptr)
    {
        if(StringUtil::EqualsIgnoreCase(assetCache->mId, extension))
        {
            return assetCache;
        }
        assetCache = assetCache->next;
    }
    return nullptr;
}
