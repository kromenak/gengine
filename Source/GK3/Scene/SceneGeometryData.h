//
// Clark Kromenaker
//
// Describes all the data associated with a scene's geometry.
// This includes: BSP geometry, BSP lightmaps, and scene w/ optional skybox & light data.
//
#pragma once
#include <string>

#include "SceneAsset.h"

class BSP;
class BSPLightmap;
class SceneAsset;
class Skybox;

class SceneGeometryData
{
public:
    void Load(const std::string& sceneAssetName);

    BSP* GetBSP() const { return mBSP; }
    Skybox* GetSkybox() const { return mSceneAsset != nullptr ? mSceneAsset->GetSkybox() : nullptr; }

private:
    // The scene asset. One *must* be defined, but really just so we can get the BSP data.
    // A skybox *may* also be defined in here.
    SceneAsset* mSceneAsset = nullptr;

    // BSP model, retrieved from the Scene asset.
    BSP* mBSP = nullptr;

    // BSP lightmap, determined from the scene asset.
    // The rule seems to be that the lightmap to be used always has the same name as the scene asset.
    BSPLightmap* mBSPLightmap = nullptr;
};