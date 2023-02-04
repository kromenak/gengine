#include "AssetManager.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "BarnFile.h"
#include "FileSystem.h"
#include "mstream.h"
#include "Services.h"
#include "StringUtil.h"

// Header Includes for all asset types
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
#include "SheepScript.h"
#include "Soundtrack.h"
#include "TextAsset.h"
#include "Texture.h"
#include "VertexAnimation.h"

AssetManager::AssetManager()
{
    // Load GK3.ini from the root directory so we can bootstrap asset search paths.
    mSearchPaths.push_back("");
    Config* config = LoadConfig("GK3.ini");
    mSearchPaths.clear();

    // The config should be present, but is technically optional.
    if(config != nullptr)
    {
        // Load "high priority" custom paths, if any.
        // These paths will be searched first to find any requested resources.
        std::string customPaths = config->GetString("Custom Paths");
        if(!customPaths.empty())
        {
            // Multiple paths are separated by semicolons.
            std::vector<std::string> paths = StringUtil::Split(customPaths, ';');
            mSearchPaths.insert(mSearchPaths.end(), paths.begin(), paths.end());
        }
    }

    // Add hard-coded default paths *after* any custom paths specified in .INI file.
    // Assets: loose files that aren't packed into a BRN.
    mSearchPaths.push_back("Assets");

    // Data: content shipped with the original game; lowest priority so assets can be easily overridden.
    mSearchPaths.push_back("Data");
}

AssetManager::~AssetManager()
{
	// All the loaded stuff has to be unloaded!
    UnloadAssets(mLoadedTexts);

    UnloadAssets(mLoadedShaders);

    UnloadAssets(mLoadedFonts);
    UnloadAssets(mLoadedCursors);

	UnloadAssets(mLoadedSheeps);

	UnloadAssets(mLoadedBSPs);
    UnloadAssets(mLoadedBSPLightmaps);
	UnloadAssets(mLoadedActionSets);
	UnloadAssets(mLoadedSceneAssets);
	UnloadAssets(mLoadedSIFs);

    UnloadAssets(mLoadedSequences);
	UnloadAssets(mLoadedVertexAnimations);
	UnloadAssets(mLoadedAnimations);
	UnloadAssets(mLoadedGases);
	
	UnloadAssets(mLoadedTextures);
	UnloadAssets(mLoadedModels);
	
	UnloadAssets(mLoadedYaks);
	UnloadAssets(mLoadedSoundtracks);
	UnloadAssets(mLoadedAudios);
	
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

bool AssetManager::LoadBarn(const std::string& barnName)
{
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(barnName) != mLoadedBarns.end()) { return true; }
    
    // Find path to barn file.
    std::string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        std::cout << "Barn doesn't exist at any search path." << std::endl;
		return false;
    }
    
    // Load barn file.
    mLoadedBarns.emplace(barnName, assetPath);
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

Audio* AssetManager::LoadAudio(const std::string& name)
{
    return LoadAsset<Audio>(SanitizeAssetName(name, ".WAV"), &mLoadedAudios, nullptr, false);
}

Soundtrack* AssetManager::LoadSoundtrack(const std::string& name)
{
    return LoadAsset<Soundtrack>(SanitizeAssetName(name, ".STK"), &mLoadedSoundtracks);
}

Animation* AssetManager::LoadYak(const std::string& name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".YAK"), &mLoadedYaks);
}

Model* AssetManager::LoadModel(const std::string& name)
{
    return LoadAsset<Model>(SanitizeAssetName(name, ".MOD"), &mLoadedModels);
}

Texture* AssetManager::LoadTexture(const std::string& name)
{
    // Load texture, attempting to add .BMP extension if asset name has no extension.
    Texture* texture = LoadAsset<Texture>(SanitizeAssetName(name, ".BMP"), &mLoadedTextures);

    //HACK: If a period exists in the asset name, it can mess up extension adding logic. (Ex: "PREP.HTOP" should resolve to "PREP.HTOP.BMP")
    //HACK: To fix this, if a texture load fails, try again forcing the BMP extension.
    if(texture == nullptr)
    {
        texture = LoadAsset<Texture>(name + ".BMP", &mLoadedTextures);
    }
    return texture;
}

Texture* AssetManager::LoadSceneTexture(const std::string& name)
{
    // Load texture per usual.
    Texture* texture = LoadTexture(name);

    // A "scene" texture means it is rendered as part of the 3D game scene (as opposed to a 2D UI texture).
    // These textures look better if you apply mipmaps and filtering.
    if(texture != nullptr && texture->GetRenderType() != Texture::RenderType::AlphaTest)
    {
        bool useMipmaps = Services::GetRenderer()->UseMipmaps();
        texture->SetMipmaps(useMipmaps);

        bool useTrilinearFiltering = Services::GetRenderer()->UseTrilinearFiltering();
        texture->SetFilterMode(useTrilinearFiltering ? Texture::FilterMode::Trilinear : Texture::FilterMode::Bilinear);
    }
    return texture;
}

GAS* AssetManager::LoadGAS(const std::string& name)
{
    return LoadAsset_SeparateLoadFunc<GAS>(SanitizeAssetName(name, ".GAS"), &mLoadedGases);
}

Animation* AssetManager::LoadAnimation(const std::string& name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".ANM"), &mLoadedAnimations);
}

Animation* AssetManager::LoadMomAnimation(const std::string& name)
{
    // GK3 has this notion of a "mother-of-all-animations" file. Thing is, it's nearly identical to a normal .ANM file...
    // Only difference I could find is MOM files support a few more keywords.
    // Anyway, it's all the same thing in my eyes!
    return LoadAsset<Animation>(SanitizeAssetName(name, ".MOM"), &mLoadedMomAnimations);
}

VertexAnimation* AssetManager::LoadVertexAnimation(const std::string& name)
{
    return LoadAsset<VertexAnimation>(SanitizeAssetName(name, ".ACT"), &mLoadedVertexAnimations);
}

Sequence* AssetManager::LoadSequence(const std::string& name)
{
    return LoadAsset<Sequence>(SanitizeAssetName(name, ".SEQ"), &mLoadedSequences);
}

SceneInitFile* AssetManager::LoadSIF(const std::string& name)
{
    return LoadAsset<SceneInitFile>(SanitizeAssetName(name, ".SIF"), &mLoadedSIFs);
}

SceneAsset* AssetManager::LoadSceneAsset(const std::string& name)
{
    return LoadAsset<SceneAsset>(SanitizeAssetName(name, ".SCN"), &mLoadedSceneAssets);
}

NVC* AssetManager::LoadNVC(const std::string& name)
{
    return LoadAsset<NVC>(SanitizeAssetName(name, ".NVC"), &mLoadedActionSets);
}

BSP* AssetManager::LoadBSP(const std::string& name)
{
    return LoadAsset<BSP>(SanitizeAssetName(name, ".BSP"), &mLoadedBSPs);
}

void AssetManager::UnloadBSP(BSP* bsp)
{
    UnloadAsset<BSP>(bsp, &mLoadedBSPs);
}

BSPLightmap* AssetManager::LoadBSPLightmap(const std::string& name)
{
    return LoadAsset<BSPLightmap>(SanitizeAssetName(name, ".MUL"), &mLoadedBSPLightmaps);
}

//TODO: For some reason, on Mac/Clang, when compiling in Release mode, using this as a Lambda with std::function causes a malloc error and crash.
//TODO: I've converted it to a normal function with a function pointer for now...which seems to work. But why?
SheepScript* LoadSheepFunc(const std::string& assetName, char* buffer, unsigned int bufferSize)
{
    // Determine whether this is a binary sheep asset.
    if(SheepScript::IsSheepDataCompiled(buffer, bufferSize))
    {
        return new SheepScript(assetName, buffer, bufferSize);
    }

    // This doesn't appear to be a binary sheep file, so it might be a text sheep file.
    // Let's try compiling it on-the-fly!
    imstream stream(buffer, bufferSize);
    return Services::GetSheep()->Compile(assetName, stream);
}

SheepScript* AssetManager::LoadSheep(const std::string& name)
{
    // Sheep assets need more complex/custom creation login, provided in the create callback.
    return LoadAsset<SheepScript>(SanitizeAssetName(name, ".SHP"), &mLoadedSheeps, &LoadSheepFunc);
}

Cursor* AssetManager::LoadCursor(const std::string& name)
{
    return LoadAsset<Cursor>(SanitizeAssetName(name, ".CUR"), &mLoadedCursors);
}

Font* AssetManager::LoadFont(const std::string& name)
{
	return LoadAsset<Font>(SanitizeAssetName(name, ".FON"), &mLoadedFonts);
}

Shader* AssetManager::LoadShader(const std::string& name)
{
    // Assumes vert/frag shaders have the same name.
    return LoadShader(name, name);
}

Shader* AssetManager::LoadShader(const std::string& vertName, const std::string& fragName)
{
    // Return existing shader if already loaded.
	std::string key = vertName + fragName;
	auto it = mLoadedShaders.find(key);
	if(it != mLoadedShaders.end())
	{
		return it->second;
	}

    // Get paths for vert/frag shaders.
	std::string vertFilePath = GetAssetPath(vertName + ".vert");
	std::string fragFilePath = GetAssetPath(fragName + ".frag");

    // Try to load shader.
	Shader* shader = new Shader(vertFilePath.c_str(), fragFilePath.c_str());
	if(shader == nullptr || !shader->IsGood())
	{
		return nullptr;
	}
	
	// Cache and return.
	mLoadedShaders[key] = shader;
	return shader;
}

TextAsset* AssetManager::LoadText(const std::string& name)
{
    // Specifically DO NOT delete the asset buffer when creating TextAssets, since they take direct ownership of it.
    return LoadAsset<TextAsset>(name, &mLoadedTexts, nullptr, false);
}

void AssetManager::UnloadText(TextAsset* text)
{
    UnloadAsset<TextAsset>(text, &mLoadedTexts);
}

Config* AssetManager::LoadConfig(const std::string& name)
{
    return LoadAsset<Config>(SanitizeAssetName(name, ".CFG"), &mLoadedConfigs);
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
	// Iterate over all loaded barn files to find the asset.
	for(auto& entry : mLoadedBarns)
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
                    std::cout << "Asset " << fileName << " exists in Barn " << (*asset->barnFileName) << ", but that Barn is not loaded!" << std::endl;
                }
                return barn;
			}
			else
			{
				return &entry.second;
			}
		}
	}
	
	// Didn't find the Barn containing this asset.
	return nullptr;
}

std::string AssetManager::SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension)
{
    // We want to add the expected extension if no extension already exists on the name.
    if(!Path::HasExtension(assetName))
    {
        return assetName + expectedExtension;
    }
    return assetName;
}

template<class T>
T* AssetManager::LoadAsset(const std::string& assetName, std::unordered_map_ci<std::string, T*>* cache, T*(*createFunc)(const std::string&, char*, unsigned int), bool deleteBuffer)
{
    // If already present in cache, return existing asset right away.
    if(cache != nullptr)
    {
        auto it = cache->find(assetName);
        if(it != cache->end())
        {
            return it->second;
        }
    }

    // Create buffer containing this asset's data. If this fails, the asset doesn't exist, so we can't load it.
    unsigned int bufferSize = 0;
    char* buffer = CreateAssetBuffer(assetName, bufferSize);
    if(buffer == nullptr) { return nullptr; }

    // Create asset from asset buffer.
    std::string upperName = StringUtil::ToUpperCopy(assetName);
    T* asset = createFunc != nullptr ? createFunc(upperName, buffer, bufferSize) : new T(upperName, buffer, bufferSize);
    
	// Add entry in cache, if we have a cache.
	if(asset != nullptr && cache != nullptr)
	{
		(*cache)[assetName] = asset;
	}

    // Delete the buffer after use (or it'll leak).
    if(deleteBuffer)
    {
        delete[] buffer;
    }
	return asset;
}

template<class T>
T* AssetManager::LoadAsset_SeparateLoadFunc(const std::string& assetName, std::unordered_map_ci<std::string, T*>* cache, bool deleteBuffer)
{
    // If already present in cache, return existing asset right away.
    if(cache != nullptr)
    {
        auto it = cache->find(assetName);
        if(it != cache->end())
        {
            return it->second;
        }
    }

    // Create buffer containing this asset's data. If this fails, the asset doesn't exist, so we can't load it.
    unsigned int bufferSize = 0;
    char* buffer = CreateAssetBuffer(assetName, bufferSize);
    if(buffer == nullptr) { return nullptr; }

    // Create asset.
    std::string upperName = StringUtil::ToUpperCopy(assetName);
    T* asset = new T(upperName);

    // If there's a cache, put the asset in the cache right away.
    // Sometimes, assets have circular depedencies, and that'll crash unless we have the item in the cache BEFORE loading!
    if(cache != nullptr)
    {
        (*cache)[assetName] = asset;
    }

    // Ok, now we can load the asset's data.
    asset->Load(buffer, bufferSize);

    // Delete the buffer after use (or it'll leak).
    if(deleteBuffer)
    {
        delete[] buffer;
    }
    return asset;
}

char* AssetManager::CreateAssetBuffer(const std::string& assetName, unsigned int& outBufferSize)
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

template<class T>
void AssetManager::UnloadAsset(T* asset, std::unordered_map_ci<std::string, T*>* cache)
{
    // Remove from cache.
    if(cache != nullptr)
    {
        auto it = cache->find(asset->GetName());
        if(it != cache->end())
        {
            cache->erase(it);
        }
    }

    // Delete asset.
    delete asset;
}

template<class T>
void AssetManager::UnloadAssets(std::unordered_map_ci<std::string, T*>& cache)
{
	// Delete all assets in the cache.
	for(auto& entry : cache)
	{
		delete entry.second;
	}
	
	// Clear the cache.
	cache.clear();
}
