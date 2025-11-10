#include "AssetNameResolver.h"

#include "FileSystem.h"

bool AssetNameResolver::HasValidExtension(const std::string& assetName) const
{
    // If an override function is provided, use that.
    if(mHasValidExtensionOverride != nullptr)
    {
        return mHasValidExtensionOverride(assetName);
    }

    // By default, consider any extension as valid.
    return Path::HasExtension(assetName);
}

void AssetNameResolver::AddTypeExtension(TypeId typeId, const std::string& extension, const std::string& assetCacheId)
{
    // The extension can't be empty.
    if(extension.empty()) { return; }

    // Make sure the extension includes the leading period.
    if(extension.front() == '.')
    {
        mExpectedAssetExtensionsByType[typeId][assetCacheId].emplace_back(extension);
    }
    else
    {
        mExpectedAssetExtensionsByType[typeId][assetCacheId].emplace_back(extension);
    }
}

const std::vector<std::string>& AssetNameResolver::GetTypeExtensions(TypeId typeId, const std::string& assetCacheId)
{
    // Get and return by type/cacheId.
    // This *can* create new empty entries if nothing existed previously. But probably OK, not going to lose much memory here.
    return mExpectedAssetExtensionsByType[typeId][assetCacheId];
}