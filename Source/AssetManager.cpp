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
    
    //TODO: Maybe load barn if not loaded?
 
    // If not loaded, we can't get it - return null.
    auto iter = mLoadedBarns.find(dictKey);
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

Texture* AssetManager::LoadTexture(string textureName)
{
    // See if this texture is already loaded.
    // If so, we can just return it right away.
    auto it = mLoadedTextures.find(textureName);
    if(it != mLoadedTextures.end())
    {
        return it->second;
    }
    std::cout << "Loading texture " << textureName << std::endl;
    
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    string assetPath = GetAssetPath(textureName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetContainingBarn(textureName);
    if(barn != nullptr)
    {
        // Extract bytes from the barn file contents.
        BarnAsset* asset = barn->GetAsset(textureName);
        char* buffer = new char[asset->uncompressedSize];
        barn->Extract(textureName, buffer, asset->uncompressedSize);
        
        // Generate texture asset from bytes.
        Texture* texture = new Texture(textureName, buffer, asset->uncompressedSize);
        mLoadedTextures[textureName] = texture;
        return texture;
    }
    
    // Couldn't find the asset!
    return nullptr;
}

BSP* AssetManager::LoadBSP(string bspName)
{
    // First, see if the asset exists at any asset search path.
    // If so, we load the asset directly from file.
    string assetPath = GetAssetPath(bspName);
    if(!assetPath.empty())
    {
        //TODO: Load asset from file.
        return nullptr;
    }
    
    // If no file to load, we'll get the asset from a barn.
    BarnFile* barn = GetContainingBarn(bspName);
    if(barn != nullptr)
    {
        BarnAsset* asset = barn->GetAsset(bspName);
        char* buffer = new char[asset->uncompressedSize];
        barn->Extract(bspName, buffer, asset->uncompressedSize);
        
        BSP* bsp = new BSP(bspName, buffer, asset->uncompressedSize);
        return bsp;
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
