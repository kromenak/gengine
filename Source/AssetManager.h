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
#include "BSPLightmap.h"
#include "Cursor.h"
#include "GAS.h"
#include "Font.h"
#include "Model.h"
#include "NVC.h"
#include "SceneAsset.h"
#include "SceneInitFile.h"
#include "Shader.h"
#include "Sheep/SheepScript.h"
#include "Soundtrack.h"
#include "Texture.h"
#include "VertexAnimation.h"

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();
	
	// Adds a filesystem path to search for assets and bundles at.
    void AddSearchPath(const std::string& searchPath);
	
	// Load or unload a barn bundle.
    bool LoadBarn(const std::string& barnName);
    void UnloadBarn(const std::string& barnName);
	
	// Write an asset from a bundle to a file.
    void WriteBarnAssetToFile(const std::string& assetName);
	void WriteBarnAssetToFile(const std::string& assetName, const std::string& outputDir);
	
	// Write all assets from a bundle that match a search string.
	void WriteAllBarnAssetsToFile(const std::string& search);
	void WriteAllBarnAssetsToFile(const std::string& search, const std::string& outputDir);
	
    Audio* LoadAudio(const std::string& name);
    Soundtrack* LoadSoundtrack(const std::string& name);
	Animation* LoadYak(const std::string& name);
    
    Model* LoadModel(const std::string& name);
    Texture* LoadTexture(const std::string& name);
    
    GAS* LoadGAS(const std::string& name);
    Animation* LoadAnimation(const std::string& name);
    VertexAnimation* LoadVertexAnimation(const std::string& name);
    
    SceneInitFile* LoadSIF(const std::string& name);
    SceneAsset* LoadSceneAsset(const std::string& name);
    NVC* LoadNVC(const std::string& name);
    
    BSP* LoadBSP(const std::string& name);
    BSPLightmap* LoadBSPLightmap(const std::string& name);
    
    SheepScript* LoadSheep(const std::string& name);
    
    Cursor* LoadCursor(const std::string& name);
	Font* LoadFont(const std::string& name);
	
    Shader* LoadShader(const std::string& name);
	Shader* LoadShader(const std::string& vertName, const std::string& fragName);
	
	char* LoadRaw(const std::string& name, unsigned int& outBufferSize);
    
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
	std::unordered_map<std::string, Animation*> mLoadedYaks;
	
	std::unordered_map<std::string, Model*> mLoadedModels;
    std::unordered_map<std::string, Texture*> mLoadedTextures;
	
	std::unordered_map<std::string, GAS*> mLoadedGases;
	std::unordered_map<std::string, Animation*> mLoadedAnimations;
	std::unordered_map<std::string, VertexAnimation*> mLoadedVertexAnimations;
	
	std::unordered_map<std::string, SceneInitFile*> mLoadedSIFs;
	std::unordered_map<std::string, SceneAsset*> mLoadedSceneAssets;
	std::unordered_map<std::string, NVC*> mLoadedActionSets;
    
	std::unordered_map<std::string, BSP*> mLoadedBSPs;
    std::unordered_map<std::string, BSPLightmap*> mLoadedBSPLightmaps;
    
	std::unordered_map<std::string, SheepScript*> mLoadedSheeps;
	
    std::unordered_map<std::string, Shader*> mLoadedShaders;
	
	// Retrieve a barn bundle by name, or by contained asset.
	BarnFile* GetBarn(const std::string& barnName);
	BarnFile* GetBarnContainingAsset(const std::string& assetName);
    
    std::string SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension);
    
    std::string GetAssetPath(const std::string& fileName);
    
    template<class T> T* LoadAsset(const std::string& assetName, std::unordered_map<std::string, T*>* cache);
	char* CreateAssetBuffer(const std::string& assetName, unsigned int& outBufferSize);
	
	template<class T> void UnloadAssets(std::unordered_map<std::string, T*>& cache);
};
