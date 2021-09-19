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

#include "SceneInitFile.h"
#include "SceneAsset.h"
#include "Timeblock.h"

struct Action;
class BSP;
class BSPLightmap;
class GKActor;
class NVC;
class Skybox;
class Soundtrack;
class WalkerBoundary;

class SceneData
{
public:
	SceneData(const std::string& location, const std::string& timeblock);
	
	// SCENE RESOLUTION
	const SceneActor* DetermineWhoEgoWillBe() const;
	void ResolveSceneData();
	
	// SCENE SETTINGS
    const Timeblock& GetTimeblock() const { return mTimeblock; }
	BSP* GetBSP() const { return mBSP; }
	Skybox* GetSkybox() const { return mSkybox; }
	WalkerBoundary* GetWalkerBoundary() const { return mWalkerBoundary; }
	const std::string& GetFloorModelName() const { return mGeneralSettings.floorModelName; }
	const std::string& GetCameraBoundsModelName() const { return mGeneralSettings.cameraBoundsModelName; }

    const Vector3& GetGlobalLightPosition() const { return mGeneralSettings.globalLightPosition; }
    const std::vector<SceneLight>& GetLights() const { return mSceneAsset->GetLights(); }

	// ACTORS/MODELS
	const std::vector<const SceneActor*>& GetActors() const { return mActors; }
	const std::vector<const SceneModel*>& GetModels() const { return mModels; }
	
	// POSITIONS
	const ScenePosition* GetScenePosition(const std::string& positionName) const;
	
	// CAMERAS
	const RoomSceneCamera* GetDefaultRoomCamera() const { return mDefaultRoomCamera; }
	const RoomSceneCamera* GetRoomCamera(const std::string& cameraName) const;
	const SceneCamera* GetCinematicCamera(const std::string& cameraName) const;
	const DialogueSceneCamera* GetDialogueCamera(const std::string& cameraName) const;
    const DialogueSceneCamera* GetInitialDialogueCameraForConversation(const std::string& conversationName) const;
    const DialogueSceneCamera* GetFinalDialogueCameraForConversation(const std::string& conversationName) const;

	// SOUNDTRACK
	Soundtrack* GetSoundtrack() const { return mSoundtracks.size() > 0 ? mSoundtracks.back() : nullptr; }

    // CONVERSATIONS
    std::vector<const SceneConversation*> GetConversationSettings(const std::string& conversationName) const;
	
private:
	Timeblock mTimeblock;
	
	// Every location *must* have a general SIF.
	// Specific SIFs, however, are optional.
	SceneInitFile* mGeneralSIF = nullptr;
	SceneInitFile* mSpecificSIF = nullptr;
	
	// The general block to be used by the scene.
	GeneralBlock mGeneralSettings;
		
	// The scene asset. One *must* be defined, but really just so we can get the BSP data.
	SceneAsset* mSceneAsset = nullptr;
	
	// BSP model, retrieved from the Scene asset.
	BSP* mBSP = nullptr;
    
    // BSP lightmap, determined from the scene asset.
    // The rule seems to be that the lightmap to use always has the same name as the scene asset.
    BSPLightmap* mBSPLightmap = nullptr;
    
	// The skybox the scene should use.
	// This can be defined in serveral spots. The priority is:
	// 1) Skybox from Specific SIF's Scene Model.
	// 2) Skybox from Specific SIF.
	// 3) Skybox from General SIF's Scene Model.
	// 4) Skybox from General SIF.
	Skybox* mSkybox = nullptr;
	
	// Walker boundary for the scene, if any.
	WalkerBoundary* mWalkerBoundary = nullptr;
	
	// Combined generic and specific actors to spawn.
	std::vector<const SceneActor*> mActors;
	
	// Combined generic and specific models to spawn.
	std::vector<const SceneModel*> mModels;
	
	// Combined generic and specific positions to use.
	std::vector<const ScenePosition*> mPositions;
	
	// Combined generic and specific cameras to use.
	std::vector<const SceneCamera*> mInspectCameras;
	std::vector<const RoomSceneCamera*> mRoomCameras;
	const RoomSceneCamera* mDefaultRoomCamera = nullptr;
	std::vector<const SceneCamera*> mCinematicCameras;
	std::vector<const DialogueSceneCamera*> mDialogueCameras;
	
	// Combined generic and specific soundtracks to use.
	// Or is there ever only one???
	std::vector<Soundtrack*> mSoundtracks;

    // Combined generic and specific conversation settings to use.
    std::vector<const SceneConversation*> mConversations;
	
	// Combined generic and specific action sets.
	// Each set contains multiple actions.
	std::vector<NVC*> mActionSets;
	
	void AddActorBlocks(const std::vector<ConditionalBlock<SceneActor>>& actorBlocks);
	void AddModelBlocks(const std::vector<ConditionalBlock<SceneModel>>& modelBlocks);
	void AddPositionBlocks(const std::vector<ConditionalBlock<ScenePosition>>& positionBlocks);
	
	void AddInspectCameraBlocks(const std::vector<ConditionalBlock<SceneCamera>>& cameraBlocks);
	void AddRoomCameraBlocks(const std::vector<ConditionalBlock<RoomSceneCamera>>& cameraBlocks);
	void AddCinematicCameraBlocks(const std::vector<ConditionalBlock<SceneCamera>>& cameraBlocks);
	void AddDialogueCameraBlocks(const std::vector<ConditionalBlock<DialogueSceneCamera>>& cameraBlocks);
	
	void AddSoundtrackBlocks(const std::vector<ConditionalBlock<Soundtrack*>>& soundtrackBlocks);
    void AddConversationBlocks(const std::vector<ConditionalBlock<SceneConversation>>& conversationBlocks);
	void AddActionBlocks(const std::vector<ConditionalBlock<NVC*>>& actionSetBlocks, bool performNameCheck);
};
