//
//  AssetManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/17/17.
//
#pragma once
#include <string>
#include <vector>

#include "Animation.h"
#include "Audio.h"
#include "BarnFile.h"
#include "BSP.h"
#include "Cursor.h"
#include "GAS.h"
#include "Font.h"
#include "Model.h"
#include "NVC.h"
#include "SceneModel.h"
#include "Shader.h"
#include "Sheep/SheepScript.h"
#include "SIF.h"
#include "Soundtrack.h"
#include "Texture.h"
#include "VertexAnimation.h"
#include "Yak.h"

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();
	
	// Adds a filesystem path to search for assets and bundles at.
    void AddSearchPath(std::string searchPath);
	
	// Load or unload a barn bundle.
    void LoadBarn(std::string barnName);
    void UnloadBarn(std::string barnName);
	
	// Write an asset from a bundle to a file.
    void WriteBarnAssetToFile(std::string assetName);
	void WriteBarnAssetToFile(std::string assetName, std::string outputDir);
	
	// Write all assets from a bundle that match a search string.
	void WriteAllBarnAssetsToFile(std::string search);
	void WriteAllBarnAssetsToFile(std::string search, std::string outputDir);
	
    Audio* LoadAudio(std::string name);
    Soundtrack* LoadSoundtrack(std::string name);
    Yak* LoadYak(std::string name);
    
    Model* LoadModel(std::string name);
    Texture* LoadTexture(std::string name);
    
    GAS* LoadGAS(std::string name);
    Animation* LoadAnimation(std::string name);
    VertexAnimation* LoadVertexAnimation(std::string name);
    
    SIF* LoadSIF(std::string name);
    SceneModel* LoadSceneModel(std::string name);
    NVC* LoadNVC(std::string name);
    BSP* LoadBSP(std::string name);
    
    SheepScript* LoadSheep(std::string name);
    
    Cursor* LoadCursor(std::string name);
	Font* LoadFont(std::string name);
	
    Shader* LoadShader(std::string name);
	Shader* LoadShader(std::string vertName, std::string fragName);
	
	char* LoadRaw(std::string name, unsigned int& outBufferSize);
    
private:
    // A list of paths to search for assets.
    // In priority order, since we'll search in order, and stop when we find the item.
    std::vector<std::string> mSearchPaths;
    
    // A map of loaded barn files. If an asset isn't found on any search path,
    // we then search each loaded barn file for the asset.
    std::unordered_map<std::string, BarnFile*> mLoadedBarns;
    
    // A list of loaded assets, so we can just return existing assets if already loaded.
    std::unordered_map<std::string, Audio*> mLoadedAudios;
	std::unordered_map<std::string, Soundtrack*> mLoadedSoundtracks;
	std::unordered_map<std::string, Yak*> mLoadedYaks;
	
	std::unordered_map<std::string, Model*> mLoadedModels;
    std::unordered_map<std::string, Texture*> mLoadedTextures;
	
	std::unordered_map<std::string, GAS*> mLoadedGases;
	std::unordered_map<std::string, Animation*> mLoadedAnimations;
	std::unordered_map<std::string, VertexAnimation*> mLoadedVertexAnimations;
	
	std::unordered_map<std::string, SIF*> mLoadedSIFs;
	std::unordered_map<std::string, SceneModel*> mLoadedSceneModels;
	std::unordered_map<std::string, NVC*> mLoadedActionSets;
	std::unordered_map<std::string, BSP*> mLoadedBSPs;
	std::unordered_map<std::string, SheepScript*> mLoadedSheeps;
	
    std::unordered_map<std::string, Shader*> mLoadedShaders;
	
	// Retrieve a barn bundle by name, or by contained asset.
	BarnFile* GetBarn(std::string barnName);
	BarnFile* GetBarnContainingAsset(std::string assetName);
    
    std::string SanitizeAssetName(std::string assetName, std::string expectedExtension);
    
    std::string GetAssetPath(std::string fileName);
    
    template<class T> T* LoadAsset(std::string assetName, std::unordered_map<std::string, T*>* cache);
	char* LoadAssetBuffer(std::string assetName, unsigned int& outBufferSize);
};
