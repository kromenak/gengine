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
#include "Texture.h"
#include "BSP.h"
#include "SIF.h"
#include "SceneData.h"
#include "NVC.h"
#include "Soundtrack.h"
#include "Yak.h"
#include "Cursor.h"
#include "VertexAnimation.h"
#include "Sheep/SheepScript.h"
#include "Shader.h"
#include "GAS.h"
#include "Animation.h"

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();
    
    void AddSearchPath(std::string searchPath);
    
    void LoadBarn(std::string barnName);
    void UnloadBarn(std::string barnName);
    BarnFile* GetBarn(std::string barnName);
    BarnFile* GetBarnContainingAsset(std::string assetName);
    void WriteBarnAssetToFile(std::string assetName);
    void WriteOutAssetsOfType(std::string extension);
    
    Audio* LoadAudio(std::string name);
    Soundtrack* LoadSoundtrack(std::string name);
    Yak* LoadYak(std::string name);
    
    Model* LoadModel(std::string name);
    Texture* LoadTexture(std::string name);
    
    GAS* LoadGAS(std::string name);
    Animation* LoadAnimation(std::string name);
    VertexAnimation* LoadVertexAnimation(std::string name);
    
    SIF* LoadSIF(std::string name);
    SceneData* LoadScene(std::string name);
    NVC* LoadNVC(std::string name);
    BSP* LoadBSP(std::string name);
    
    SheepScript* LoadSheep(std::string name);
    
    Cursor* LoadCursor(std::string name);
    
    Shader* LoadShader(std::string name);
    
private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    std::vector<std::string> mSearchPaths;
    
    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    std::unordered_map<std::string, BarnFile*> mLoadedBarns;
    
    // A list of loaded assets, so we can just return existing assets if already loaded.
    std::unordered_map<std::string, Audio*> mLoadedAudios;
    std::unordered_map<std::string, Texture*> mLoadedTextures;
    
    std::unordered_map<std::string, Shader*> mLoadedShaders;
    
    std::string SanitizeAssetName(std::string assetName, std::string expectedExtension);
    
    std::string GetAssetPath(std::string fileName);
    
    template<class T> T* LoadAsset(std::string assetName, std::unordered_map<std::string, T*>* cache);
};
