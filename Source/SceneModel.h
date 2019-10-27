//
// SceneModel.h
//
// Clark Kromenaker
//
// A "scene model" asset. Mainly indicates BSP and light positions for a scene.
//
// In-memory representation of a .SCN asset.
//
#pragma once
#include "Asset.h"

class Skybox;

class SceneModel : public Asset
{
public:
    SceneModel(std::string name, char* data, int dataLength);
	~SceneModel();
	
	const std::string& GetBSPName() const { return mBspName; }
    Skybox* GetSkybox() const { return mSkybox; }
    
private:
    // Often, the BSP name is equal to the SCN name. But if not, it's here.
    std::string mBspName;
    
    // A skybox to use for the scene. This might also be specified in the SIF.
    Skybox* mSkybox = nullptr;
    
    void ParseFromData(char* data, int dataLength);
};
