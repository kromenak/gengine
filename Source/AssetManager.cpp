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

void AssetManager::AddSearchPath(string searchPath)
{
    // If the search path already exists in the list, don't add it again.
    if(find(mSearchPaths.begin(), mSearchPaths.end(), searchPath) != mSearchPaths.end())
    {
        return;
    }
    mSearchPaths.push_back(searchPath);
}

void AssetManager::LoadBarn(string barnName)
{
    // We want our dictionary key to be all uppercase.
    // Avoids confusion with case-sensitive key searches.
    string dictKey = barnName;
    StringUtil::ToUpper(dictKey);
    
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(dictKey) != mLoadedBarns.end()) { return; }
    
    // Find path to barn file.
    string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        cout << "Barn doesn't exist at any search path." << endl;
        return;
    }
    
    // Load barn file.
    BarnFile* barn = new BarnFile(assetPath.c_str());
    mLoadedBarns[dictKey] = barn;
}

void AssetManager::UnloadBarn(string barnName)
{
    // We want our dictionary key to be all uppercase.
    string dictKey = barnName;
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

BarnFile* AssetManager::GetBarn(string barnName)
{
    // We want our dictionary key to be all uppercase.
    string dictKey = barnName;
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

Audio* AssetManager::LoadAudio(string name)
{
    return LoadAsset<Audio>(name, &mLoadedAudios);
}

Soundtrack* AssetManager::LoadSoundtrack(string name)
{
    return LoadAsset<Soundtrack>(name, nullptr);
}

Model* AssetManager::LoadModel(string name)
{
    return LoadAsset<Model>(name, nullptr);
}

Texture* AssetManager::LoadTexture(string name)
{
    return LoadAsset<Texture>(name, &mLoadedTextures);
}

SIF* AssetManager::LoadSIF(string name)
{
    return LoadAsset<SIF>(name, nullptr);
}

Scene* AssetManager::LoadScene(string name)
{
    return LoadAsset<Scene>(name, nullptr);
}

NVC* AssetManager::LoadNVC(string name)
{
    return LoadAsset<NVC>(name, nullptr);
}

BSP* AssetManager::LoadBSP(string name)
{
    return LoadAsset<BSP>(name, nullptr);
}

string AssetManager::GetAssetPath(string fileName)
{
    for(const string& searchPath : mSearchPaths)
    {
        string path = searchPath + fileName;
        
        ifstream f(path.c_str());
        if(f.good()) { return path; }
    }
    return "";
}

BarnFile* AssetManager::GetContainingBarn(string fileName)
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

template<class T>
T* AssetManager::LoadAsset(string assetName, unordered_map<string, T*>* cache)
{
    string upperName = assetName;
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
    string assetPath = GetAssetPath(upperName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetContainingBarn(upperName);
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
