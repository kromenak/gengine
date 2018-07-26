//
//  AssetManager.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//
#include "AssetManager.h"
#include <iostream>
#include "StringUtil.h"

AssetManager::AssetManager()
{
    
}

AssetManager::~AssetManager()
{
    // Delete and clear all loaded barns.
    for(auto& entry : mLoadedBarns)
    {
        delete entry.second;
    }
    mLoadedBarns.clear();
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
    return nullptr;
}

void AssetManager::WriteBarnAssetToFile(std::string assetName)
{
    BarnFile* barn = GetBarnContainingAsset(assetName);
    if(barn != nullptr)
    {
        barn->WriteToFile(assetName);
    }
}

void AssetManager::WriteOutAssetsOfType(std::string extension)
{
    for(auto& entry : mLoadedBarns)
    {
        entry.second->WriteAllOfType(extension);
    }
}

Audio* AssetManager::LoadAudio(std::string name)
{
    return LoadAsset<Audio>(SanitizeAssetName(name, ".WAV"), &mLoadedAudios);
}

Soundtrack* AssetManager::LoadSoundtrack(std::string name)
{
    return LoadAsset<Soundtrack>(SanitizeAssetName(name, ".STK"), nullptr);
}

Yak* AssetManager::LoadYak(std::string name)
{
    return LoadAsset<Yak>(SanitizeAssetName(name, ".YAK"), nullptr);
}

Model* AssetManager::LoadModel(std::string name)
{
    return LoadAsset<Model>(SanitizeAssetName(name, ".MOD"), nullptr);
}

Texture* AssetManager::LoadTexture(std::string name)
{
    return LoadAsset<Texture>(SanitizeAssetName(name, ".BMP"), &mLoadedTextures);
}

GAS* AssetManager::LoadGAS(std::string name)
{
    return LoadAsset<GAS>(SanitizeAssetName(name, ".GAS"), nullptr);
}

Animation* AssetManager::LoadAnimation(std::string name)
{
    return LoadAsset<Animation>(SanitizeAssetName(name, ".ANM"), nullptr);
}

ACT* AssetManager::LoadACT(std::string name)
{
    return LoadAsset<ACT>(SanitizeAssetName(name, ".ACT"), nullptr);
}

SIF* AssetManager::LoadSIF(std::string name)
{
    return LoadAsset<SIF>(SanitizeAssetName(name, ".SIF"), nullptr);
}

SceneData* AssetManager::LoadScene(std::string name)
{
    return LoadAsset<SceneData>(SanitizeAssetName(name, ".SCN"), nullptr);
}

NVC* AssetManager::LoadNVC(std::string name)
{
    return LoadAsset<NVC>(SanitizeAssetName(name, ".NVC"), nullptr);
}

BSP* AssetManager::LoadBSP(std::string name)
{
    return LoadAsset<BSP>(SanitizeAssetName(name, ".BSP"), nullptr);
}

SheepScript* AssetManager::LoadSheep(std::string name)
{
    return LoadAsset<SheepScript>(SanitizeAssetName(name, ".SHP"), nullptr);
}

Cursor* AssetManager::LoadCursor(std::string name)
{
    return LoadAsset<Cursor>(SanitizeAssetName(name, ".CUR"), nullptr);
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
    if(shader->IsGood())
    {
        mLoadedShaders[name] = shader;
    }
    return shader;
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
    for(const std::string& searchPath : mSearchPaths)
    {
        std::string path = searchPath + fileName;
        
        std::ifstream f(path.c_str());
        if(f.good()) { return path; }
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
    
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    std::string assetPath = GetAssetPath(upperName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetBarnContainingAsset(upperName);
    if(barn != nullptr)
    {
        // Extract bytes from the barn file contents.
        BarnAsset* barnAsset = barn->GetAsset(upperName);
        char* buffer = new char[barnAsset->uncompressedSize];
        barn->Extract(upperName, buffer, barnAsset->uncompressedSize);
        
        // Generate asset from the BARN bytes.
        T* asset = new T(upperName, buffer, barnAsset->uncompressedSize);
        
        // Add entry in cache, if we have a cache.
        if(cache != nullptr)
        {
            (*cache)[upperName] = asset;
        }
        
        //std::cout << "Loaded asset " << upperName << std::endl;
        return asset;
    }
    
    // Couldn't find the asset!
    std::cout << "Asset " << upperName << " could not be loaded!" << std::endl;
    return nullptr;
}
