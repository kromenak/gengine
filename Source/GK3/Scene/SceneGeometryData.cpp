#include "SceneGeometryData.h"

#include "AssetManager.h"
#include "BSP.h"
#include "BSPLightmap.h"

void SceneGeometryData::Load(const std::string& sceneAssetName)
{
    // Load the desired scene asset - chosen based on settings block.
    mSceneAsset = gAssetManager.LoadAsset<SceneAsset>(sceneAssetName, AssetScope::Scene);

    // Load the BSP data, which is specified by the scene model.
    // If this is null, the game will still work...but there's no BSP geometry!
    if(mSceneAsset != nullptr)
    {
        mBSP = gAssetManager.LoadAsset<BSP>(mSceneAsset->GetBSPName(), AssetScope::Scene);
    }
    else
    {
        mBSP = gAssetManager.LoadAsset<BSP>("DEFAULT.BSP");
    }

    // Load BSP lightmap data.
    mBSPLightmap = gAssetManager.LoadAsset<BSPLightmap>(sceneAssetName, AssetScope::Scene);

    // Apply lightmap to BSP.
    if(mBSP != nullptr && mBSPLightmap != nullptr)
    {
        mBSP->ApplyLightmap(*mBSPLightmap);
    }
}