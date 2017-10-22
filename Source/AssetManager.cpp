//
//  AssetManager.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//
#include "AssetManager.h"
#include <iostream>

AssetManager::AssetManager()
{
    
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
    // If the barn is already in the map, then we don't need to load it again.
    if(mLoadedBarns.find(barnName) != mLoadedBarns.end()) { return; }
    
    string assetPath = GetAssetPath(barnName);
    if(assetPath.empty())
    {
        cout << "Barn doesn't exist at any search path." << endl;
        return;
    }
    
    BarnFile* barn = new BarnFile(assetPath.c_str());
    mLoadedBarns[barnName] = barn;
}

void AssetManager::UnloadBarn(string barnName)
{
    // If the barn isn't in the map, we can't unload it!
    auto iter = mLoadedBarns.find(barnName);
    if(iter == mLoadedBarns.end()) { return; }

    //TODO: Do we need to unload all loaded assets from this barn?
    
    // Delete barn.
    BarnFile* barn = iter->second;
    delete barn;
    
    // Remove from map.
    mLoadedBarns.erase(barnName);
}

BarnFile* AssetManager::GetBarn(string barnName)
{
    //TODO: Maybe load barn if not loaded?
 
    // If not loaded, we can't get it - return null.
    auto iter = mLoadedBarns.find(barnName);
    if(iter == mLoadedBarns.end()) { return nullptr; }
    
    // Found it!
    return iter->second;
}

Audio* AssetManager::LoadAudio(string audioName)
{
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    string assetPath = GetAssetPath(audioName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetContainingBarn(audioName);
    if(barn != nullptr)
    {
        BarnAsset* asset = barn->GetAsset(audioName);
        char* buffer = new char[asset->uncompressedSize];
        barn->Extract(audioName, buffer, asset->uncompressedSize);
        
        Audio* audio = new Audio(audioName, buffer, asset->uncompressedSize);
        return audio;
    }
    
    // Couldn't find this asset!
    return nullptr;
}

Model* AssetManager::LoadModel(string modelName)
{
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    string assetPath = GetAssetPath(modelName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetContainingBarn(modelName);
    if(barn != nullptr)
    {
        BarnAsset* asset = barn->GetAsset(modelName);
        char* buffer = new char[asset->uncompressedSize];
        barn->Extract(modelName, buffer, asset->uncompressedSize);
        
        Model* model = new Model(modelName, buffer, asset->uncompressedSize);
        return model;
    }
    
    // Couldn't find this asset!
    return nullptr;

}

string AssetManager::GetAssetPath(string fileName)
{
    for(const string& searchPath : mSearchPaths)
    {
        string path = searchPath + fileName;
        ifstream f(path.c_str());
        if(f.good())
        {
            return path;
        }
    }
    return "";
}

BarnFile* AssetManager::GetContainingBarn(string fileName)
{
    for(const auto& entry : mLoadedBarns)
    {
        BarnAsset* asset = entry.second->GetAsset(fileName);
        if(asset != nullptr)
        {
            return entry.second;
        }
    }
    return nullptr;
}

BarnAsset* AssetManager::GetBarnAsset(string fileName)
{
    for(const auto& entry : mLoadedBarns)
    {
        BarnAsset* asset = entry.second->GetAsset(fileName);
        if(asset != nullptr)
        {
            return asset;
        }
    }
    return nullptr;
}
