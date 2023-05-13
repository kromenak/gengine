//
// Clark Kromenaker
//
// Sheep SysFuncs related to manipulating game assets.
//
#pragma once
#include "SheepSysFunc.h"

// ASSET PATHS
shpvoid AddPath(const std::string& pathName); // DEV
shpvoid FullScanPaths(); // DEV
shpvoid RescanPaths(); // DEV

// ASSET EXTRACTION
shpvoid Extract(const std::string& fileSpec, const std::string& outputPath);
shpvoid ExtractResourceDir(const std::string& outputFileName);

shpvoid SaveSprite(const std::string& spriteName, const std::string& fileName); // DEV
shpvoid SaveTexture(const std::string& textureName, const std::string& fileName); // DEV
shpvoid SaveTextureX(const std::string& textureName, int surfaceIndex, const std::string& fileName); // DEV

// ASSET LOADING/UNLOADING
shpvoid UnloadAll(); // DEV
shpvoid UnloadAllAnimations(); // DEV
shpvoid UnloadAllModels(); // DEV
shpvoid UnloadAllMovies(); // DEV
shpvoid UnloadAllScenes(); // DEV
shpvoid UnloadAllSounds(); // DEV
shpvoid UnloadAllSprites(); // DEV
shpvoid UnloadAllTextures(); // DEV

shpvoid UnloadAnimation(const std::string& animName); // DEV
shpvoid UnloadModel(const std::string& modelName); // DEV
shpvoid UnloadMovie(const std::string& movieName); // DEV
shpvoid UnloadScene(const std::string& sceneName); // DEV
shpvoid UnloadSound(const std::string& soundName); // DEV
shpvoid UnloadSprite(const std::string& spriteName); // DEV
shpvoid UnloadTexture(const std::string& textureName); // DEV

// ASSET EDITING
shpvoid Edit(const std::string& fileName); // DEV
shpvoid Open(const std::string& fileName); // DEV
shpvoid OpenFile(const std::string& fileName); // DEV

// MISC
shpvoid NeedDiscResources(int discNum);
