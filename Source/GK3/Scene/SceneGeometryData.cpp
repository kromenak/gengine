#include "SceneGeometryData.h"

#include "AssetManager.h"
#include "BSP.h"

void SceneGeometryData::Load(const std::string& sceneAssetName)
{
    // Load the desired scene asset - chosen based on settings block.
    mSceneAsset = gAssetManager.LoadSceneAsset(sceneAssetName, AssetScope::Scene);

    // Load the BSP data, which is specified by the scene model.
    // If this is null, the game will still work...but there's no BSP geometry!
    if(mSceneAsset != nullptr)
    {
        mBSP = gAssetManager.LoadBSP(mSceneAsset->GetBSPName(), AssetScope::Scene);
    }
    else
    {
        mBSP = gAssetManager.LoadBSP("DEFAULT.BSP");
    }

    // Load BSP lightmap data.
    mBSPLightmap = gAssetManager.LoadBSPLightmap(sceneAssetName, AssetScope::Scene);

    // Apply lightmap to BSP.
    if(mBSP != nullptr && mBSPLightmap != nullptr)
    {
        mBSP->ApplyLightmap(*mBSPLightmap);
    }
}
