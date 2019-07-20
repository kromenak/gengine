//
// SceneData.h
//
// Clark Kromenaker
//
// The logic in GK3 for what scene assets should be loaded and used for
// particular timeblocks is not entirely straightforward. Each location has
// a "general" SIF, plus sometimes a "specific" SIF for current location/timeblock.
//
// Also, assets referenced by the general/specific SIFs have particular rules
// for when to load, and what overrides what, if both SIFs define the same key.
// For example, the skybox can be defined in either SIF, or in either SIF's scene model!
//
// This class takes care of sorting out the above logic, so the Scene itself
// can just worry about using the data, and not deal with loading and override intricacies.
//
#pragma once
#include <string>
#include <vector>

#include "SIF.h"

class BSP;
class GKActor;
class NVC;
class NVCItem;
class SceneModel;
class Skybox;
class Soundtrack;
class WalkerBoundary;

class SceneData
{
public:
	SceneData(std::string location, std::string timeblock);
	
	BSP* GetBSP() const { return mBSP; }
	std::string GetFloorModelName() const;
	Skybox* GetSkybox() const { return mSkybox; }
	
	SceneCameraData* GetDefaultRoomCamera() const;
	SceneCameraData* GetRoomCamera(std::string cameraName) const;
	
	ScenePositionData* GetScenePosition(std::string positionName) const;
	
	WalkerBoundary* GetWalkerBoundary() const;
	
	std::vector<SceneActorData*> GetSceneActorDatas() const { return mSceneActorDatas; }
	std::vector<SceneModelData*> GetSceneModelDatas() const { return mSceneModelDatas; }
	
	Soundtrack* GetSoundtrack() const { return mSoundtrack; }
	
	std::vector<NVC*> GetNounVerbCaseSets() const { return mNounVerbCaseSets; }
	std::vector<const NVCItem*> GetViableVerbsForNoun(std::string noun, GKActor* ego) const;
	const NVCItem* GetNounVerbAction(std::string noun, std::string verb, GKActor* ego) const;
	
private:
	// Every location *must* have a general SIF.
	// Specific SIFs, however, are optional.
	SIF* mGeneralSIF = nullptr;
	SIF* mSpecificSIF = nullptr;
	
	// The scene model. One *must* be defined, but really
	// just so we can get the BSP data.
	SceneModel* mSceneModel = nullptr;
	
	// BSP model, retrieved from the Scene Model.
	BSP* mBSP = nullptr;
	
	// The skybox the scene should use.
	// This can be defined in serveral spots. The priority is:
	// 1) Skybox from Specific SIF's Scene Model.
	// 2) Skybox from Specific SIF.
	// 3) Skybox from General SIF's Scene Model.
	// 4) Skybox from General SIF.
	Skybox* mSkybox = nullptr;
	
	// Combined generic and specific scene actor datas.
	std::vector<SceneActorData*> mSceneActorDatas;
	
	// Combined generic and specific scene model datas.
	std::vector<SceneModelData*> mSceneModelDatas;
	
	// Combined generic and specific NVC sets.
	std::vector<NVC*> mNounVerbCaseSets;
	
	// The soundtrack to use.
	Soundtrack* mSoundtrack = nullptr;
};
