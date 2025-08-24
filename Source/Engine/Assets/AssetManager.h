//
// Clark Kromenaker
//
// Manages loading and caching of assets.
//
#pragma once
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

#include "Asset.h"
#include "AssetCache.h"
#include "BarnFile.h"
#include "StringUtil.h"

// Forward Declarations for all asset types
class Animation;
class Audio;
class BSP;
class BSPLightmap;
class Config;
class Cursor;
class Font;
class GAS;
class Model;
class NVC;
class SceneAsset;
class SceneInitFile;
class Sequence;
class Shader;
class SheepScript;
class Soundtrack;
class TextAsset;
class Texture;
class VertexAnimation;

class AssetManager
{
public:
    void Init();
    void Shutdown();

    // Loose Files
    // Adds a filesystem path to search for assets and bundles at.
    void AddSearchPath(const std::string& searchPath);

    // Given a filename, finds the path to the file if it exists on one of the search paths.
    // Returns empty string if file is not found.
    std::string GetAssetPath(const std::string& fileName);
    std::string GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions);

    // Barn Files
    // Load or unload a barn bundle.
    bool LoadBarn(const std::string& barnName, BarnSearchPriority priority = BarnSearchPriority::Normal);
    void UnloadBarn(const std::string& barnName);

    // Write an asset from a bundle to a file.
    void WriteBarnAssetToFile(const std::string& assetName);
    void WriteBarnAssetToFile(const std::string& assetName, const std::string& outputDir);

    // Write all assets from a bundle that match a search string.
    void WriteAllBarnAssetsToFile(const std::string& search);
    void WriteAllBarnAssetsToFile(const std::string& search, const std::string& outputDir);

    // Loading (or Getting) Assets
    Audio* LoadAudio(const std::string& name, AssetScope scope = AssetScope::Global);
    Audio* LoadAudioAsync(const std::string& name, AssetScope scope = AssetScope::Global);
    Soundtrack* LoadSoundtrack(const std::string& name, AssetScope scope = AssetScope::Global);
    Animation* LoadYak(const std::string& name, AssetScope scope = AssetScope::Global);

    Model* LoadModel(const std::string& name, AssetScope scope = AssetScope::Global);
    Texture* LoadTexture(const std::string& name, AssetScope scope = AssetScope::Global);
    Texture* LoadTextureAsync(const std::string& name, AssetScope scope = AssetScope::Global);
    Texture* LoadSceneTexture(const std::string& name, AssetScope scope = AssetScope::Global);
    const std::string_map_ci<Texture*>& GetLoadedTextures() const { return mTextureCache.cache; }

    GAS* LoadGAS(const std::string& name, AssetScope scope = AssetScope::Global);
    Animation* LoadAnimation(const std::string& name, AssetScope scope = AssetScope::Global);
    Animation* LoadMomAnimation(const std::string& name, AssetScope scope = AssetScope::Global);
    VertexAnimation* LoadVertexAnimation(const std::string& name, AssetScope scope = AssetScope::Global);
    Sequence* LoadSequence(const std::string& name, AssetScope scope = AssetScope::Global);

    SceneInitFile* LoadSIF(const std::string& name, AssetScope scope = AssetScope::Global);
    SceneAsset* LoadSceneAsset(const std::string& name, AssetScope scope = AssetScope::Global);
    NVC* LoadNVC(const std::string& name, AssetScope scope = AssetScope::Global);

    BSP* LoadBSP(const std::string& name, AssetScope scope = AssetScope::Global);
    BSPLightmap* LoadBSPLightmap(const std::string& name, AssetScope scope = AssetScope::Global);

    SheepScript* LoadSheep(const std::string& name, AssetScope scope = AssetScope::Global);

    Cursor* LoadCursor(const std::string& name, AssetScope scope = AssetScope::Global);
    Cursor* LoadCursorAsync(const std::string& name, AssetScope scope = AssetScope::Global);
    Font* LoadFont(const std::string& name, AssetScope scope = AssetScope::Global);

    TextAsset* LoadText(const std::string& name, AssetScope scope = AssetScope::Global);
    Config* LoadConfig(const std::string& name);

    Shader* GetShader(const std::string& id);
    Shader* LoadShader(const std::string& idToUse, const std::string& vertexShaderFileNameNoExt, const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags);
    Shader* LoadShader(const std::string& idToUse, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags);

    // Unloading Assets
    void UnloadAssets(AssetScope scope);

    // Querying Assets
    template<typename T> T* GetOrLoadAsset(const std::string& name, AssetScope scope = AssetScope::Global, const std::string& id = "");
    template<typename T> const std::string_map_ci<T*>* GetLoadedAssets(const std::string& id = "");

private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    std::vector<std::string> mSearchPaths;

    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    std::string_map_ci<BarnFile> mLoadedBarns;

    // Tracks the highest priority we've seen for a loaded Barn.
    // This just helps us be a bit more efficient (e.g. don't bother searching High priority if no high priority Barns even exist).
    BarnSearchPriority mHighestBarnSearchPriority = BarnSearchPriority::Low;

    // A list of loaded assets, so we can just return existing assets if already loaded.
    AssetCache<Audio> mAudioCache;
    AssetCache<Soundtrack> mSoundtrackCache;
    AssetCache<Animation> mYakCache { "yak" };

    AssetCache<Model> mModelCache;
    AssetCache<Texture> mTextureCache;

    AssetCache<Animation> mAnimationCache { "anm" };
    AssetCache<Animation> mMomAnimationCache { "mom" };
    AssetCache<Sequence> mSequenceCache;
    AssetCache<VertexAnimation> mVertexAnimationCache;
    AssetCache<GAS> mGasCache;

    AssetCache<SceneInitFile> mSifCache;
    AssetCache<SceneAsset> mSceneAssetCache;
    AssetCache<NVC> mNvcCache;

    AssetCache<BSP> mBspCache;
    AssetCache<BSPLightmap> mBspLightmapCache;

    AssetCache<SheepScript> mSheepCache;

    AssetCache<Cursor> mCursorCache;
    AssetCache<Font> mFontCache;

    AssetCache<TextAsset> mTextAssetCache;
    AssetCache<Config> mConfigCache;

    AssetCache<Shader> mShaderCache;

    // Retrieve a barn bundle by name, or by contained asset.
    BarnFile* GetBarn(const std::string& barnName);
    BarnFile* GetBarnContainingAsset(const std::string& assetName);

    std::string SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension);

    // Two ways to load an asset:
    // The first uses a single constructor (name, data, size).
    // The second uses a constructor (name) and a separate load function (data, size).
    // The latter is necessary if two assets can potentially attempt to load one another (circular dependency).
    template<typename T> T* LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache, bool deleteBuffer = true);
    template<typename T> T* LoadAssetAsync(const std::string& name, AssetScope scope, AssetCache<T>* cache, bool deleteBuffer = true, std::function<void(T*)> callback = nullptr);

    uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize);

    template<class T> void UnloadAsset(T* asset, std::unordered_map_ci<std::string, T*>* cache = nullptr);
};

extern AssetManager gAssetManager;

template<typename T>
inline T* AssetManager::GetOrLoadAsset(const std::string& name, AssetScope assetScope, const std::string& assetCacheId)
{
    AssetCache<T>* assetCache = nullptr;
    T* asset = nullptr;

    // Attempt to retrieve an asset cache for this asset type.
    AssetCacheBase* baseAssetCache = AssetCacheBase::GetAssetCache(T::StaticTypeId(), assetCacheId);
    if(baseAssetCache != nullptr)
    {
        assetCache = dynamic_cast<AssetCache<T>*>(baseAssetCache);

        // If the asset cache exists, try to retrieve the desired asset from the cache.
        if(assetCache != nullptr)
        {
            asset = assetCache->Get(name);
        }
    }

    // If asset is null, attempt to load it and put it in the asset cache (if any).
    if(asset == nullptr)
    {
        asset = LoadAsset<T>(name, assetScope, assetCache);
    }
    return asset;
}

template<typename T>
const std::string_map_ci<T*>* AssetManager::GetLoadedAssets(const std::string& id)
{
    AssetCacheBase* baseAssetCache = AssetCacheBase::GetAssetCache(T::StaticTypeId(), id);
    if(baseAssetCache != nullptr)
    {
        AssetCache<T>* assetCache = dynamic_cast<AssetCache<T>*>(baseAssetCache);
        if(assetCache != nullptr)
        {
            return &assetCache->cache;
        }
    }
    return nullptr;
}
