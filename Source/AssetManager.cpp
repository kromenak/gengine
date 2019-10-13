//
//  AssetManager.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//
#include "AssetManager.h"

#include <fstream>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h>

#include "StringUtil.h"

AssetManager::AssetManager()
{
    
}

AssetManager::~AssetManager()
{
	// All the loaded stuff has to be unloaded!
	UnloadAssets(mLoadedShaders);
	
	UnloadAssets(mLoadedSheeps);
	UnloadAssets(mLoadedBSPs);
	UnloadAssets(mLoadedActionSets);
	UnloadAssets(mLoadedSceneModels);
	UnloadAssets(mLoadedSIFs);
	
	UnloadAssets(mLoadedVertexAnimations);
	UnloadAssets(mLoadedAnimations);
	UnloadAssets(mLoadedGases);
	
	UnloadAssets(mLoadedTextures);
	UnloadAssets(mLoadedModels);
	
	UnloadAssets(mLoadedYaks);
	UnloadAssets(mLoadedSoundtracks);
	UnloadAssets(mLoadedAudios);
	
	UnloadAssets(mLoadedBarns);
}

void AssetManager::AddSearchPath(std::string searchPath)
{
    // If the search path already exists in the list, don't add it again.
    if(find(mSearchPaths.begin(), mSearchPaths.end(), searchPath) != mSearchPaths.end())
    {
        return;
    }
    mSearchPaths.push_back(searchPath);
}

void AssetManager::LoadBarn(std::string barnName)
{
    // We want our dictionary key to be all uppercase.
    // Avoids confusion with case-sensitive key searches.
    std::string dictKey = barnName;
    StringUtil::ToUpper(dictKey);
    
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(dictKey) != mLoadedBarns.end()) { return; }
    
    // Find path to barn file.
    std::string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        std::cout << "Barn doesn't exist at any search path." << std::endl;
        return;
    }
    
    // Load barn file.
    BarnFile* barn = new BarnFile(assetPath.c_str());
    mLoadedBarns[dictKey] = barn;
}

void AssetManager::UnloadBarn(std::string barnName)
{
    // We want our dictionary key to be all uppercase.
    std::string dictKey = barnName;
    StringUtil::ToUpper(dictKey);
    
    // If the barn isn't in the map, we can't unload it!
    auto iter = mLoadedBarns.find(dictKey);
    if(iter == mLoadedBarns.end()) { return; }
    
    // Delete barn.
    BarnFile* barn = iter->second;
    delete barn;
    
    // Remove from map.
    mLoadedBarns.erase(dictKey);
}

void AssetManager::WriteBarnAssetToFile(std::string assetName)
{
	WriteBarnAssetToFile(assetName, "");
}

void AssetManager::WriteBarnAssetToFile(std::string assetName, std::string outputDir)
{
	BarnFile* barn = GetBarnContainingAsset(assetName);
	if(barn != nullptr)
	{
		barn->WriteToFile(assetName, outputDir);
	}
}

void AssetManager::WriteAllBarnAssetsToFile(std::string search)
{
	WriteAllBarnAssetsToFile(search, "");
}

void AssetManager::WriteAllBarnAssetsToFile(std::string search, std::string outputDir)
{
	// Pass the buck to all loaded barn files.
	for(auto& entry : mLoadedBarns)
	{
		entry.second->WriteAllToFile(search, outputDir);
	}
}

Audio* AssetManager::LoadAudio(std::string name)
{
    return LoadAsset<Audio>(SanitizeAssetName(name, ".WAV"), &mLoadedAudios);
}

Soundtrack* AssetManager::LoadSoundtrack(std::string name)
{
    return LoadAsset<Soundtrack>(SanitizeAssetName(name, ".STK"), &mLoadedSoundtracks);
}

Yak* AssetManager::LoadYak(std::string name)
{
    return LoadAsset<Yak>(SanitizeAssetName(name, ".YAK"), &mLoadedYaks);
}

Model* AssetManager::LoadModel(std::string name)
{
    return LoadAsset<Model>(SanitizeAssetName(name, ".MOD"), &mLoadedModels);
}

Texture* AssetManager::LoadTexture(std::string name)
{
    return LoadAsset<Texture>(SanitizeAssetName(name, ".BMP"), &mLoadedTextures);
}

GAS* AssetManager::LoadGAS(std::string name)
{
    return LoadAsset<GAS>(SanitizeAssetName(name, ".GAS"), &mLoadedGases);
}

Animation* AssetManager::LoadAnimation(std::string name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".ANM"), &mLoadedAnimations);
}

VertexAnimation* AssetManager::LoadVertexAnimation(std::string name)
{
    return LoadAsset<VertexAnimation>(SanitizeAssetName(name, ".ACT"), &mLoadedVertexAnimations);
}

SIF* AssetManager::LoadSIF(std::string name)
{
    return LoadAsset<SIF>(SanitizeAssetName(name, ".SIF"), &mLoadedSIFs);
}

SceneModel* AssetManager::LoadSceneModel(std::string name)
{
    return LoadAsset<SceneModel>(SanitizeAssetName(name, ".SCN"), &mLoadedSceneModels);
}

NVC* AssetManager::LoadNVC(std::string name)
{
    return LoadAsset<NVC>(SanitizeAssetName(name, ".NVC"), &mLoadedActionSets);
}

BSP* AssetManager::LoadBSP(std::string name)
{
    return LoadAsset<BSP>(SanitizeAssetName(name, ".BSP"), &mLoadedBSPs);
}

SheepScript* AssetManager::LoadSheep(std::string name)
{
    return LoadAsset<SheepScript>(SanitizeAssetName(name, ".SHP"), &mLoadedSheeps);
}

Cursor* AssetManager::LoadCursor(std::string name)
{
    return LoadAsset<Cursor>(SanitizeAssetName(name, ".CUR"), nullptr);
}

Font* AssetManager::LoadFont(std::string name)
{
	return LoadAsset<Font>(SanitizeAssetName(name, ".FON"), nullptr);
}

Shader* AssetManager::LoadShader(std::string name)
{
    auto it = mLoadedShaders.find(name);
    if(it != mLoadedShaders.end())
    {
        return it->second;
    }
    
    std::string vertFilePath = GetAssetPath(name + ".vert");
    std::string fragFilePath = GetAssetPath(name + ".frag");
    
    Shader* shader = new Shader(vertFilePath.c_str(), fragFilePath.c_str());
	
	// If shader couldn't be found, or failed to load for some reason, return null.
	if(shader == nullptr || !shader->IsGood())
	{
		return nullptr;
	}
	
	// Cache and return.
	mLoadedShaders[name] = shader;
    return shader;
}

Shader* AssetManager::LoadShader(std::string vertName, std::string fragName)
{
	std::string key = vertName + fragName;
	auto it = mLoadedShaders.find(key);
	if(it != mLoadedShaders.end())
	{
		return it->second;
	}
	
	std::string vertFilePath = GetAssetPath(vertName + ".vert");
	std::string fragFilePath = GetAssetPath(fragName + ".frag");
	
	Shader* shader = new Shader(vertFilePath.c_str(), fragFilePath.c_str());
	
	// If shader couldn't be found, or failed to load for some reason, return null.
	if(shader == nullptr || !shader->IsGood())
	{
		return nullptr;
	}
	
	// Cache and return.
	mLoadedShaders[key] = shader;
	return shader;
}

char* AssetManager::LoadRaw(std::string name, unsigned int& outBufferSize)
{
	return LoadAssetBuffer(name, outBufferSize);
}

BarnFile* AssetManager::GetBarn(std::string barnName)
{
	// We want our dictionary key to be all uppercase.
	std::string dictKey = barnName;
	StringUtil::ToUpper(dictKey);
	
	// If we find it, return it.
	auto iter = mLoadedBarns.find(dictKey);
	if(iter != mLoadedBarns.end())
	{
		return iter->second;
	}
	
	//TODO: Maybe load barn if not loaded?
	return nullptr;
}

BarnFile* AssetManager::GetBarnContainingAsset(std::string fileName)
{
	// Iterate over all loaded barn files to find the asset.
	for(const auto& entry : mLoadedBarns)
	{
		BarnAsset* asset = entry.second->GetAsset(fileName);
		if(asset != nullptr)
		{
			// If the asset is a pointer, we need to redirect to the correct BarnFile.
			// If the correct Barn isn't available, spit out an error and fail.
			if(asset->IsPointer())
			{
				auto it = mLoadedBarns.find(asset->barnFileName);
				if(it != mLoadedBarns.end())
				{
					return it->second;
				}
				else
				{
					std::cout << "Asset " << fileName << " exists in Barn " << asset->barnFileName << ", but that Barn is not loaded!" << std::endl;
				}
			}
			else
			{
				return entry.second;
			}
		}
	}
	
	// Didn't find the Barn containing this asset.
	return nullptr;
}

std::string AssetManager::SanitizeAssetName(std::string assetName, std::string expectedExtension)
{
    // First, convert all names to uppercase.
    std::string sanitizedName = assetName;
    StringUtil::ToUpper(sanitizedName);
    
    // We want to add the expected extension if it isn't present, and no other extension is present.
    // There's probably a better way to do this...
    if(!expectedExtension.empty() && sanitizedName.find(expectedExtension) == std::string::npos)
    {
        if(sanitizedName.size() < 4 || sanitizedName[sanitizedName.size() - 4] != '.')
        {
            sanitizedName += expectedExtension;
        }
    }
    return sanitizedName;
}

std::string AssetManager::GetAssetPath(std::string fileName)
{
	CFBundleRef bundleRef = CFBundleGetMainBundle();
	if(bundleRef != nullptr)
	{
		for(const std::string& searchPath : mSearchPaths)
		{
			CFStringRef searchPathCFStr = CFStringCreateWithCString(CFAllocatorGetDefault(), searchPath.c_str(), kCFStringEncodingUTF8);
			CFStringRef fileNameCFStr = CFStringCreateWithCString(CFAllocatorGetDefault(), fileName.c_str(), kCFStringEncodingUTF8);
			CFURLRef assetUrl = CFBundleCopyResourceURL(bundleRef, fileNameCFStr, NULL, searchPathCFStr);
			
			if(assetUrl != nullptr)
			{
				CFStringRef assetStr = CFURLGetString(assetUrl);
				std::string path = CFStringGetCStringPtr(assetStr, kCFStringEncodingUTF8);
				path.erase(0, 7);
			
				std::ifstream f(path.c_str());
				if(f.good()) { return path; }
			}
		}
	}
	else
	{
		for(const std::string& searchPath : mSearchPaths)
		{
			std::string path = searchPath + fileName;
			
			std::ifstream f(path.c_str());
			if(f.good()) { return path; }
		}
	}
    return "";
}

template<class T>
T* AssetManager::LoadAsset(std::string assetName, std::unordered_map<std::string, T*>* cache)
{
    std::string upperName = assetName;
    StringUtil::ToUpper(upperName);
    
    // See if this asset is already loaded in the cache
    // If so, we can just return it right away.
    if(cache != nullptr)
    {
        auto it = cache->find(upperName);
        if(it != cache->end())
        {
            return it->second;
        }
    }
	
	// Retrieve the buffer, from which we'll create the asset.
	unsigned int bufferSize = 0;
	char* buffer = LoadAssetBuffer(upperName, bufferSize);
	
	// If no buffer could be found, we're in trouble!
	if(buffer == nullptr)
	{
		std::cout << "Asset " << upperName << " could not be loaded!" << std::endl;
		return nullptr;
	}
	
	// Generate asset from the BARN bytes.
	T* asset = new T(upperName, buffer, bufferSize);
	
	// Add entry in cache, if we have a cache.
	if(cache != nullptr)
	{
		(*cache)[upperName] = asset;
	}
        
	//std::cout << "Loaded asset " << upperName << std::endl;
	return asset;
}

char* AssetManager::LoadAssetBuffer(std::string assetName, unsigned int& outBufferSize)
{
	// First, see if the asset exists at any asset search path.
	// If so, we load the asset directly from file.
	std::string assetPath = GetAssetPath(assetName);
	if(!assetPath.empty())
	{
		//TODO: Load asset from file.
		return nullptr;
	}
	
	// If no file to load, we'll get the asset from a barn.
	BarnFile* barn = GetBarnContainingAsset(assetName);
	if(barn != nullptr)
	{
		// Extract bytes from the barn file contents.
		BarnAsset* barnAsset = barn->GetAsset(assetName);
		//TODO: Assert barnAsset != null.
		
		// Create a buffer of the correct size.
		outBufferSize = barnAsset->uncompressedSize;
		char* buffer = new char[outBufferSize];
		
		// Extract the asset to that buffer.
		barn->Extract(assetName, buffer, outBufferSize);
		
		// Return the buffer.
		return buffer;
	}
	
	// Couldn't find this asset!
	return nullptr;
}

template<class T>
void AssetManager::UnloadAssets(std::unordered_map<std::string, T*>& cache)
{
	// Delete all assets in the cache.
	for(auto& entry : cache)
	{
		delete entry.second;
	}
	
	// Clear the cache.
	cache.clear();
}
