#include "AssetManager.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "FileSystem.h"
#include "Localizer.h"
#include "Renderer.h"
#include "SheepManager.h"
#include "StringUtil.h"
#include "ThreadPool.h"

// Includes for all asset types
#include "Animation.h"
#include "Audio.h"
#include "BSP.h"
#include "BSPLightmap.h"
#include "Config.h"
#include "Cursor.h"
#include "Font.h"
#include "GAS.h"
#include "Model.h"
#include "NVC.h"
#include "SceneAsset.h"
#include "SceneInitFile.h"
#include "Sequence.h"
#include "Shader.h"
#include "Soundtrack.h"
#include "TextAsset.h"
#include "Texture.h"
#include "VertexAnimation.h"

AssetManager gAssetManager;

void AssetManager::Init()
{
    // Load GK3.ini from the root directory so we can bootstrap asset search paths.
    mSearchPaths.emplace_back("");
    Config* config = LoadConfig("GK3.ini");
    mSearchPaths.clear();

    // The config should be present, but is technically optional.
    if(config != nullptr)
    {
        // Load "high priority" custom paths, if any.
        // These paths will be searched first to find any requested resources.
        std::string customPaths = config->GetString("Custom Paths", "");
        if(!customPaths.empty())
        {
            // Multiple paths are separated by semicolons.
            std::vector<std::string> paths = StringUtil::Split(customPaths, ';');
            mSearchPaths.insert(mSearchPaths.end(), paths.begin(), paths.end());
        }
    }

    // Add hard-coded default paths *after* any custom paths specified in .INI file.
    // Assets: loose files that aren't packed into a BRN.
    mSearchPaths.emplace_back("Assets");

    // Data: content shipped with the original game; lowest priority so assets can be easily overridden.
    {
        // The original game only ever shipped with one language per SKU, so there was no way to change the language after install.
        // But we would like to support that maybe, for both official and unofficial translations.
        // To support OFFICIAL translations, we'll use Data folders with a suffix equal to the language prefix (e.g. DataF for French, DataG for German).
        if(Localizer::GetLanguagePrefix()[0] != 'E')
        {
            mSearchPaths.push_back("Data" + Localizer::GetLanguagePrefix());
        }

        // Lowest priority is the normal "Data" folder.
        mSearchPaths.emplace_back("Data");
    }

    // Also allow searching the root directory for assets moving forward, but at the lowest priority.
    mSearchPaths.emplace_back("");
}

void AssetManager::Shutdown()
{
    // Unload all assets.
    UnloadAssets(AssetScope::Global);

    // Clear all loaded barns.
    mLoadedBarns.clear();
}

void AssetManager::AddSearchPath(const std::string& searchPath)
{
    // If the search path already exists in the list, don't add it again.
    if(std::find(mSearchPaths.begin(), mSearchPaths.end(), searchPath) != mSearchPaths.end())
    {
        return;
    }
    mSearchPaths.push_back(searchPath);
}

std::string AssetManager::GetAssetPath(const std::string& fileName)
{
    // We have a set of paths, at which we should search for the specified filename.
    // So, iterate each search path and see if the file is in that folder.
    std::string assetPath;
    for(auto& searchPath : mSearchPaths)
    {
        if(Path::FindFullPath(fileName, searchPath, assetPath))
        {
            return assetPath;
        }
    }
    return std::string();
}

std::string AssetManager::GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions)
{
    // If already has an extension, just use the normal path find function.
    if(Path::HasExtension(fileName))
    {
        return GetAssetPath(fileName);
    }

    // Otherwise, we have a filename, but multiple valid extensions.
    // A good example is a movie file. The file might be called "intro", but the extension could be "avi" or "bik".
    for(const std::string& extension : extensions)
    {
        std::string assetPath = GetAssetPath(fileName + "." + extension);
        if(!assetPath.empty())
        {
            return assetPath;
        }
    }
    return std::string();
}

bool AssetManager::LoadBarn(const std::string& barnName, BarnSearchPriority priority)
{
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(barnName) != mLoadedBarns.end()) { return true; }

    // Find path to barn file.
    std::string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        return false;
    }

    // Remember if this is the highest search priority we've seen.
    if(priority > mHighestBarnSearchPriority)
    {
        mHighestBarnSearchPriority = priority;
    }

    // Load barn file.
    mLoadedBarns.emplace(std::piecewise_construct, std::forward_as_tuple(barnName), std::forward_as_tuple(assetPath, priority));
    return true;
}

void AssetManager::UnloadBarn(const std::string& barnName)
{
    // If the barn isn't in the map, we can't unload it!
    auto iter = mLoadedBarns.find(barnName);
    if(iter == mLoadedBarns.end()) { return; }

    // Remove from map.
    mLoadedBarns.erase(iter);
}

void AssetManager::WriteBarnAssetToFile(const std::string& assetName)
{
    WriteBarnAssetToFile(assetName, "");
}

void AssetManager::WriteBarnAssetToFile(const std::string& assetName, const std::string& outputDir)
{
    BarnFile* barn = GetBarnContainingAsset(assetName);
    if(barn != nullptr)
    {
        barn->WriteToFile(assetName, outputDir);
    }
}

void AssetManager::WriteAllBarnAssetsToFile(const std::string& search)
{
    WriteAllBarnAssetsToFile(search, "");
}

void AssetManager::WriteAllBarnAssetsToFile(const std::string& search, const std::string& outputDir)
{
    // Pass the buck to all loaded barn files.
    for(auto& entry : mLoadedBarns)
    {
        entry.second.WriteAllToFile(search, outputDir);
    }
}

Audio* AssetManager::LoadAudio(const std::string& name, AssetScope scope)
{
    return LoadAsset<Audio>(SanitizeAssetName(name, ".WAV"), scope, AssetCache<Audio>::Get(), false);
}

Soundtrack* AssetManager::LoadSoundtrack(const std::string& name, AssetScope scope)
{
    return LoadAsset<Soundtrack>(SanitizeAssetName(name, ".STK"), scope, AssetCache<Soundtrack>::Get());
}

Animation* AssetManager::LoadYak(const std::string& name, AssetScope scope)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".YAK"), scope, AssetCache<Animation>::Get("yak"));
}

Model* AssetManager::LoadModel(const std::string& name, AssetScope scope)
{
    return LoadAsset<Model>(SanitizeAssetName(name, ".MOD"), scope, AssetCache<Model>::Get());
}

Texture* AssetManager::LoadTexture(const std::string& name, AssetScope scope)
{
    return LoadAsset<Texture>(SanitizeAssetName(name, ".BMP"), scope, AssetCache<Texture>::Get());
}

Texture* AssetManager::LoadSceneTexture(const std::string& name, AssetScope scope)
{
    // Load texture per usual.
    Texture* texture = LoadTexture(name, scope);

    // A "scene" texture means it is rendered as part of the 3D game scene (as opposed to a 2D UI texture).
    // These textures look better if you apply mipmaps and filtering.
    if(texture != nullptr && texture->GetRenderType() != Texture::RenderType::AlphaTest)
    {
        bool useMipmaps = gRenderer.UseMipmaps();
        texture->SetMipmaps(useMipmaps);

        bool useTrilinearFiltering = gRenderer.UseTrilinearFiltering();
        texture->SetFilterMode(useTrilinearFiltering ? Texture::FilterMode::Trilinear : Texture::FilterMode::Bilinear);
    }

    // For some reason, many transparent scene textures in GK3 (mostly foliage) have a single non-transparent pixel at (1, 0).
    // This pixel is obviously supposed to be transparent when rendered.
    if(texture != nullptr && texture->GetRenderType() == Texture::RenderType::AlphaTest)
    {
        if(texture->GetPixelColor(0, 0) == Color32::Magenta &&
           texture->GetPixelColor(2, 0) == Color32::Magenta &&
           texture->GetPixelColor(1, 1) == Color32::Magenta)
        {
            texture->SetPixelColor(1, 0, Color32::Magenta);
        }
    }
    return texture;
}

GAS* AssetManager::LoadGAS(const std::string& name, AssetScope scope)
{
    return LoadAsset<GAS>(SanitizeAssetName(name, ".GAS"), scope, AssetCache<GAS>::Get());
}

Animation* AssetManager::LoadAnimation(const std::string& name, AssetScope scope)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".ANM"), scope, AssetCache<Animation>::Get("anm"));
}

Animation* AssetManager::LoadMomAnimation(const std::string& name, AssetScope scope)
{
    // GK3 has this notion of a "mother-of-all-animations" file. Thing is, it's nearly identical to a normal .ANM file...
    // Only difference I could find is MOM files support a few more keywords.
    // Anyway, it's all the same thing in my eyes!
    return LoadAsset<Animation>(SanitizeAssetName(name, ".MOM"), scope, AssetCache<Animation>::Get("mom"));
}

VertexAnimation* AssetManager::LoadVertexAnimation(const std::string& name, AssetScope scope)
{
    return LoadAsset<VertexAnimation>(SanitizeAssetName(name, ".ACT"), scope, AssetCache<VertexAnimation>::Get());
}

Sequence* AssetManager::LoadSequence(const std::string& name, AssetScope scope)
{
    return LoadAsset<Sequence>(SanitizeAssetName(name, ".SEQ"), scope, AssetCache<Sequence>::Get());
}

SceneInitFile* AssetManager::LoadSIF(const std::string& name, AssetScope scope)
{
    return LoadAsset<SceneInitFile>(SanitizeAssetName(name, ".SIF"), scope, AssetCache<SceneInitFile>::Get());
}

SceneAsset* AssetManager::LoadSceneAsset(const std::string& name, AssetScope scope)
{
    return LoadAsset<SceneAsset>(SanitizeAssetName(name, ".SCN"), scope, AssetCache<SceneAsset>::Get());
}

NVC* AssetManager::LoadNVC(const std::string& name, AssetScope scope)
{
    return LoadAsset<NVC>(SanitizeAssetName(name, ".NVC"), scope, AssetCache<NVC>::Get());
}

BSP* AssetManager::LoadBSP(const std::string& name, AssetScope scope)
{
    return LoadAsset<BSP>(SanitizeAssetName(name, ".BSP"), scope, AssetCache<BSP>::Get());
}

BSPLightmap* AssetManager::LoadBSPLightmap(const std::string& name, AssetScope scope)
{
    return LoadAsset<BSPLightmap>(SanitizeAssetName(name, ".MUL"), scope, AssetCache<BSPLightmap>::Get());
}

SheepScript* AssetManager::LoadSheep(const std::string& name, AssetScope scope)
{
    return LoadAsset<SheepScript>(SanitizeAssetName(name, ".SHP"), scope, AssetCache<SheepScript>::Get());
}

Cursor* AssetManager::LoadCursor(const std::string& name, AssetScope scope)
{
    return LoadAsset<Cursor>(SanitizeAssetName(name, ".CUR"), scope, AssetCache<Cursor>::Get());
}

Font* AssetManager::LoadFont(const std::string& name, AssetScope scope)
{
    return LoadAsset<Font>(SanitizeAssetName(name, ".FON"), scope, AssetCache<Font>::Get());
}

TextAsset* AssetManager::LoadText(const std::string& name, AssetScope scope)
{
    // Specifically DO NOT delete the asset buffer when creating TextAssets, since they take direct ownership of it.
    return LoadAsset<TextAsset>(name, scope, AssetCache<TextAsset>::Get(), false);
}

TextAsset* AssetManager::LoadLocalizedText(const std::string& name, AssetScope scope)
{
    TextAsset* textFile = LoadText(Localizer::GetLanguagePrefix() + name, scope);
    if(textFile == nullptr)
    {
        printf("Failed to load %s%s - falling back on English (E%s).\n", Localizer::GetLanguagePrefix().c_str(), name.c_str(), name.c_str());
        textFile = LoadText("E" + name, scope);
        if(textFile == nullptr)
        {
            printf("Failed to load localized text %s!\n", name.c_str());
        }
    }
    return textFile;
}

Config* AssetManager::LoadConfig(const std::string& name)
{
    return LoadAsset<Config>(SanitizeAssetName(name, ".CFG"), AssetScope::Global, AssetCache<Config>::Get());
}

Shader* AssetManager::GetShader(const std::string& id)
{
    // Attempt to return a cached shader.
    return AssetCache<Shader>::Get()->GetAsset(id);
}

Shader* AssetManager::LoadShader(const std::string& idToUse, const std::string& vertexShaderFileNameNoExt,
                                 const std::string& fragmentShaderFileNameNoExt, const std::vector<std::string>& featureFlags)
{
    // If shader by this name is already loaded, return that.
    Shader* cachedShader = GetShader(idToUse);
    if(cachedShader != nullptr)
    {
        return cachedShader;
    }

    // Otherwise, create a new shader.
    Shader* shader = new Shader(idToUse, vertexShaderFileNameNoExt, fragmentShaderFileNameNoExt, featureFlags);

    // If not valid, delete and return null. Log should display compiler error.
    if(!shader->IsValid())
    {
        delete shader;
        return nullptr;
    }

    // Cache and return.
    AssetCache<Shader>::Get()->SetAsset(idToUse, shader);
    return shader;
}

Shader* AssetManager::LoadShader(const std::string& idToUse, const std::string& shaderFileNameNoExt, const std::vector<std::string>& featureFlags)
{
    // Very similar to above, but assumes both vertex and fragment shader are stored in a single source file.
    // Return cached shader if one exists.
    Shader* cachedShader = GetShader(idToUse);
    if(cachedShader != nullptr)
    {
        return cachedShader;
    }

    // Create new shader.
    Shader* shader = new Shader(idToUse, shaderFileNameNoExt, featureFlags);

    // If not valid, delete and return null. Log should display compiler error.
    if(!shader->IsValid())
    {
        delete shader;
        return nullptr;
    }

    // Cache and return.
    AssetCache<Shader>::Get()->SetAsset(idToUse, shader);
    return shader;
}

void AssetManager::UnloadAssets(AssetScope scope)
{
    for(auto& entry : IAssetCache::sAssetCachesByType)
    {
        for(auto& assetCache : entry.second)
        {
            assetCache->UnloadAssets(scope);
        }
    }
}

BarnFile* AssetManager::GetBarn(const std::string& barnName)
{
    // If we find it, return it.
    auto iter = mLoadedBarns.find(barnName);
    if(iter != mLoadedBarns.end())
    {
        return &iter->second;
    }

    //TODO: Maybe load barn if not loaded?
    return nullptr;
}

BarnFile* AssetManager::GetBarnContainingAsset(const std::string& fileName)
{
    // Starting at the highest priority for loaded Barn files, try to find the asset.
    BarnSearchPriority priority = mHighestBarnSearchPriority;
    while(priority >= BarnSearchPriority::Low)
    {
        for(auto& entry : mLoadedBarns)
        {
            // If this Barn doesn't match the priority we're currently on, skip it for now.
            if(entry.second.GetSearchPriority() == priority)
            {
                BarnAsset* asset = entry.second.GetAsset(fileName);
                if(asset != nullptr)
                {
                    // If the asset is a pointer, we need to redirect to the correct BarnFile.
                    // If the correct Barn isn't available, spit out an error and fail.
                    if(asset->IsPointer())
                    {
                        BarnFile* barn = GetBarn(*asset->barnFileName);
                        if(barn == nullptr)
                        {
                            std::cout << "Asset " << fileName << " exists in Barn " << (*asset->barnFileName) << ", but that Barn is not loaded!\n";
                        }
                        return barn;
                    }
                    else
                    {
                        return &entry.second;
                    }
                }
            }
        }

        // We didn't find the asset in any barn at this priority.
        // Decrement to the next lowest priority.
        priority = static_cast<BarnSearchPriority>(static_cast<int>(priority) - 1);
    }

    // Didn't find the Barn containing this asset.
    return nullptr;
}

std::string AssetManager::SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension)
{
    // If a three-letter extension already exists, accept it and assume the caller knows what they're doing.
    // Only for 3-letter extensions! GK3 actually includes periods in a few asset names, but never with a three letter ending.
    int lastIndex = assetName.size() - 1;
    if(lastIndex > 3 && assetName[lastIndex - 3] == '.')
    {
        return assetName;
    }

    // No three-letter extension, add the expected extension if missing.
    if(!Path::HasExtension(assetName, expectedExtension))
    {
        return assetName + expectedExtension;
    }
    return assetName;
}

template<typename T>
T* AssetManager::LoadAsset(const std::string& name, AssetScope scope, AssetCache<T>* cache, bool deleteBuffer)
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
    uint32_t bufferSize = 0;
    uint8_t* buffer = CreateAssetBuffer(name, bufferSize);
    if(buffer == nullptr) { return nullptr; }

    // Create asset from asset buffer.
    std::string upperName = StringUtil::ToUpperCopy(name);
    T* asset = new T(upperName, scope);

    // Add entry in cache, if we have a cache.
    if(asset != nullptr && cache != nullptr && scope != AssetScope::Manual)
    {
        cache->SetAsset(name, asset);
    }

    // Load the asset.
    asset->Load(buffer, bufferSize);

    // Delete the buffer after use (or it'll leak).
    if(deleteBuffer)
    {
        delete[] buffer;
    }
    return asset;
}

uint8_t* AssetManager::CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize)
{
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    // Loose files take precedence over packaged barn assets.
    std::string assetPath = GetAssetPath(assetName);
    if(!assetPath.empty())
    {
        return File::ReadIntoBuffer(assetPath, outBufferSize);
    }

    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetBarnContainingAsset(assetName);
    if(barn != nullptr)
    {
        return barn->CreateAssetBuffer(assetName, outBufferSize);
    }

    // Couldn't find this asset!
    return nullptr;
}