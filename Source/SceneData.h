//
// SceneData.h
//
// Clark Kromenaker
//
// An asset that mostly just indicates the BSP and light positions for
// a scene in the game.
//
// In-memory representation of a .SCN asset.
//
#pragma once
#include "Asset.h"

class Skybox;

class SceneData : public Asset
{
public:
    SceneData(std::string name, char* data, int dataLength);
    
    std::string GetBSPName();
    
    Skybox* GetSkybox() { return mSkybox; }
    
private:
    // Often, the BSP name is equal to the SCN name. But if not, it's here.
    std::string mBSPNameOverride;
    
    // A skybox to use for the scene. This might also be specified in the SIF.
    Skybox* mSkybox = nullptr;
    
    void ParseFromData(char* data, int dataLength);
};
