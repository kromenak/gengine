//
// Clark Kromenaker
//
// Provides a mapping from localization keys to localized text.
//
#pragma once
#include <string>
#include <unordered_map>

#include "Asset.h"
#include "AssetManager.h"
#include "StringUtil.h"

class TextAsset;

class Localizer
{
public:
    static const std::string& GetLanguagePrefix();
    template<typename T> static T* LoadLocalizedAsset(const std::string& name, AssetScope scope = AssetScope::Global, const std::string& assetCacheId = "");

    Localizer() = default;
    explicit Localizer(const std::string& fileName, const std::string& sectionName = "");

    void Load(const std::string& fileName, const std::string& sectionName = "");

    bool HasText(const std::string& key) const;
    std::string GetText(const std::string& key) const;

private:
    // Maps a localization key to the localized text.
    std::string_map_ci<std::string> mKeyToText;
};

template <typename T>
T* Localizer::LoadLocalizedAsset(const std::string& name, AssetScope scope, const std::string& assetCacheId)
{
    T* asset = gAssetManager.LoadAsset<T>(GetLanguagePrefix() + name, scope, assetCacheId);
    if(asset == nullptr)
    {
        printf("Failed to load %s%s - falling back on English (E%s).\n", GetLanguagePrefix().c_str(), name.c_str(), name.c_str());
        asset = gAssetManager.LoadAsset<T>("E" + name, scope, assetCacheId);
        if(asset == nullptr)
        {
            printf("Failed to load localized asset %s!\n", name.c_str());
        }
    }
    return asset;
}

// It's possible to create many Localizer instances, but there is also one "main" localizer.
extern Localizer gLocalizer;