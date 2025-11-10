//
// Clark Kromenaker
//
// Asset names may be passed without an extension, assuming that the system will "figure it out" and load the right thing.
// For example, we might say LoadAsset<Texture>("Chessboard") and the system needs to figure out it should load "Chessboard.bmp".
//
// This class helps resolve such ambiguities by providing a way to detect if an asset has a valid extension, and if not,
// providing a way to map asset types to one or more valid extensions for that asset type.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "StringUtil.h"
#include "TypeId.h"

class AssetNameResolver
{
public:
    void SetHasValidExtensionCallback(const std::function<bool(const std::string&)>& callback) { mHasValidExtensionOverride = callback; }
    bool HasValidExtension(const std::string& assetName) const;

    template<typename T> void AddTypeExtension(const std::string& extension, const std::string& assetCacheId = "")
    {
        AddTypeExtension(T::StaticTypeId(), extension, assetCacheId);
    }
    void AddTypeExtension(TypeId typeId, const std::string& extension, const std::string& assetCacheId = "");

    template<typename T> const std::vector<std::string>& GetTypeExtensions(const std::string& assetCacheId = "")
    {
        return GetTypeExtensions(T::StaticTypeId(), assetCacheId);
    }
    const std::vector<std::string>& GetTypeExtensions(TypeId typeId, const std::string& assetCacheId = "");

private:
    // If set, defines an alternative function for deciding if an asset name already contains a valid extension or not.
    std::function<bool(const std::string&)> mHasValidExtensionOverride = nullptr;

    // Maps an asset type (and optional subtype/cacheId) to a set of expected extensions.
    // This allows us to guess valid asset names if an extension isn't provided.
    std::unordered_map<TypeId, std::string_map_ci<std::vector<std::string>>> mExpectedAssetExtensionsByType;
};
