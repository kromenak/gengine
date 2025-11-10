#include "AssetCache.h"

std::unordered_map<TypeId, std::vector<IAssetCache*>> IAssetCache::sAssetCachesByType;
std::mutex IAssetCache::sAssetCachesMutex;