//
// Clark Kromenaker
//
// A "scene" asset. Mainly indicates BSP and light positions for a scene.
// The term "scene" is a bit overloaded...but this is just a small text asset that defines some geometry features of the scene.
//
// In-memory representation of a .SCN asset.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>

#include "Color32.h"
#include "Vector3.h"

class Skybox;

/*
// SCN assets often have fairly complex lighting definitions in them.
// However, after experimenting with the original game, as far as I can tell, they are not used at all for lighting at runtime.
// Perhaps they were used for lightmap generation?
struct SceneLight
{
    std::string name;
    int type = 0;
    Vector3 position;
    Vector3 direction;
    Color32 color;
    float hotspot = 0.0f;
    float falloff = 0.0f;
    float attenuationStart = 0.0f;
    float attenuationEnd = 0.0f;
    bool useAttenuation = false;
    bool castShadows = false;
    bool overshoot = false;
    float intensity = 1.0f;
    float radius = 1.0f;
    int decayType = 0;
};
*/

class SceneAsset : public Asset
{
public:
    SceneAsset(const std::string& name, char* data, int dataLength);
	~SceneAsset();
	
	const std::string& GetBSPName() const { return mBspName; }
    Skybox* GetSkybox() const { return mSkybox; }
    
private:
    // Often, the BSP name is equal to the SCN name.
    std::string mBspName;
    
    // A skybox to use for the scene. This might also be specified in the SIF.
    Skybox* mSkybox = nullptr;

    // Lights defined for this scene.
    //std::vector<SceneLight> mLights;
    
    void ParseFromData(char* data, int dataLength);
};
