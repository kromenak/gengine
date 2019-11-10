//
// SceneInitFile.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include "Asset.h"

#include <vector>

#include "SIF.h" // For scene data structs

struct GeneralBlock
{
	SheepScript* condition = nullptr;
	
	std::string sceneAssetName;

	std::string floorModelName;
	std::string walkerBoundaryTextureName;
	Vector2 walkerBoundarySize;
	Vector2 walkerBoundaryOffset;
	
	std::string cameraBoundsModelName;
	bool cameraBoundsDynamic = false;
	
	Vector3 globalLightPosition;
	Vector3 globalLightAmbient;
	
	std::string skyboxLeftTextureName;
	std::string skyboxRightTextureName;
	std::string skyboxBackTextureName;
	std::string skyboxFrontTextureName;
	std::string skyboxDownTextureName;
	std::string skyboxUpTextureName;
	
	Skybox* CreateSkybox();
	void TakeOverridesFrom(const GeneralBlock& other);
};

template<typename T>
struct ConditionalBlock
{
	SheepScript* condition = nullptr;
	std::vector<T> items;
};

class SceneInitFile : public Asset
{
public:
	SceneInitFile(const std::string& name, char* data, int dataLength);
	~SceneInitFile();
	
	const SceneActor* FindCurrentEgo() const;
	GeneralBlock FindCurrentGeneralBlock() const;
	
	const std::vector<ConditionalBlock<SceneActor>>& GetActorBlocks() const { return mActors; }
	
	const std::vector<ConditionalBlock<SceneModel>>& GetModelBlocks() const { return mModels; }
	
	const std::vector<ConditionalBlock<ScenePosition>>& GetPositionBlocks() const { return mPositions; }
	
	const std::vector<ConditionalBlock<SceneCamera>>& GetInspectCameraBlocks() const { return mInspectCameras; }
	const std::vector<ConditionalBlock<RoomSceneCamera>>& GetRoomCameraBlocks() const { return mRoomCameras; }
	const std::vector<ConditionalBlock<SceneCamera>>& GetCinematicCameraBlocks() const { return mCinematicCameras; }
	const std::vector<ConditionalBlock<DialogueSceneCamera>>& GetDialogueCameraBlocks() const { return mDialogueCameras; }
	
	const std::vector<ConditionalBlock<SceneRegionOrTrigger>>& GetRegionBlocks() const { return mRegions; }
	const std::vector<ConditionalBlock<SceneRegionOrTrigger>>& GetTriggerBlocks() const { return mTriggers; }
	
	const std::vector<ConditionalBlock<Soundtrack*>>& GetSoundtrackBlocks() const { return mSoundtracks; }
	
	const std::vector<ConditionalBlock<NVC*>>& GetActionBlocks() const { return mActions; }
	
private:
	// GENERAL
	std::vector<GeneralBlock> mGeneralBlocks;
	
	// ACTORS
	std::vector<ConditionalBlock<SceneActor>> mActors;
	
	// MODELS
    std::vector<ConditionalBlock<SceneModel>> mModels;
	
	// POSITIONS
	std::vector<ConditionalBlock<ScenePosition>> mPositions;
	
	// CAMERAS
	std::vector<ConditionalBlock<SceneCamera>> mInspectCameras;
	std::vector<ConditionalBlock<RoomSceneCamera>> mRoomCameras;
	std::vector<ConditionalBlock<SceneCamera>> mCinematicCameras;
    std::vector<ConditionalBlock<DialogueSceneCamera>> mDialogueCameras;
	
	// REGIONS & TRIGGERS
    std::vector<ConditionalBlock<SceneRegionOrTrigger>> mRegions;
    std::vector<ConditionalBlock<SceneRegionOrTrigger>> mTriggers;
	
	// AMBIENT (aka AUDIO)
	// This one's gotta be pointers b/c a Soundtrack is a straight up Asset.
    std::vector<ConditionalBlock<Soundtrack*>> mSoundtracks;
	
	// ACTIONS
	// This one's also pointers b/c NVCs are Assets.
    std::vector<ConditionalBlock<NVC*>> mActions;
	
	void ParseFromData(char* data, int dataLength);
};
