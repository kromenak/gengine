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
class Action;
class SceneModel;
class Skybox;
class Soundtrack;
class WalkerBoundary;

class SceneData
{
public:
	SceneData(const std::string& location, const std::string& timeblock);
	
	BSP* GetBSP() const { return mBSP; }
	Skybox* GetSkybox() const { return mSkybox; }
	
	const SceneCameraData* GetDefaultRoomCamera() const;
	const SceneCameraData* GetRoomCamera(const std::string& cameraName) const;
	
	const ScenePositionData* GetScenePosition(const std::string& positionName) const;
	
	const std::string& GetFloorModelName() const;
	WalkerBoundary* GetWalkerBoundary() const;
	
	const std::vector<SceneActorData*>& GetSceneActorDatas() const { return mSceneActorDatas; }
	const std::vector<SceneModelData*>& GetSceneModelDatas() const { return mSceneModelDatas; }
	
	Soundtrack* GetSoundtrack() const { return mSoundtrack; }
	
	const std::vector<NVC*>& GetActionSets() const { return mActionSets; }
	std::vector<const Action*> GetActions(const std::string& noun, GKActor* ego) const;
	const Action* GetAction(const std::string& noun, const std::string& verb, GKActor* ego) const;
	
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
	std::vector<NVC*> mActionSets;
	
	// The soundtrack to use.
	Soundtrack* mSoundtrack = nullptr;
};
