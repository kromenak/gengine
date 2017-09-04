//
//  AssetManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//

#pragma once
#include <string>
#include <vector>
#include "BarnFile.h"
#include "Audio.h"
#include "Model.h"

using namespace std;

class AssetManager
{
public:
    
private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    vector<string> mSearchPaths;
    
    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    unordered_map<string, BarnFile*> mLoadedBarns;
    
public:
    AssetManager();
    
    void AddSearchPath(string searchPath);
    
    void LoadBarn(string barnName);
    void UnloadBarn(string barnName);
    BarnFile* GetBarn(string barnName);
    
    Audio* LoadAudio(string audioName);
    Model* LoadModel(string modelName);
    
private:
    string GetAssetPath(string fileName);
    BarnFile* GetContainingBarn(string fileName);
    BarnAsset* GetBarnAsset(string fileName);
};
