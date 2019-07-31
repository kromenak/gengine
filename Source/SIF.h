//
// SIF.h
//
// Clark Kromenaker
//
// A (S)cene (I)nitialization (F)ile, which is an asset used to load
// and initialize a scene (or room) in the game. There are "general" SIFs,
// which are used for a location at any time, and "specific" SIFs, which are
// only loaded when you go to a location during a specific timeblock.
//
// In-memory representation of .SIF files.
//
#pragma once
#include "Asset.h"

#include <vector>

#include "Color32.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"

class Animation;
class GAS;
class Model;
class NVC;
class Skybox;
class Soundtrack;
class Texture;
class WalkerBoundary;

struct SceneCameraData
{
    // The label for this camera.
    // This can be an identifier, or (for an inspect camera) the noun/model associated.
    std::string label;
    
    // Camera's angle - yaw and pitch.
    Vector2 angle;
    
    // Camera's position.
    Vector3 position;
};

struct DialogueSceneCameraData : public SceneCameraData
{
    // Specifies name of dialogue for which this camera is used.
    std::string dialogueName;
    
    // Specifies name of "set" this camera belongs to.
    // Allows logic to specify a "set" name, which randomly picks a camera in that set.
    std::string setName;
    
    // If true, this camera appears in the toolbar when the conversation is active.
    bool showInToolbar = false;
    
    // If true, camera is used when the conversation ends.
    bool isFinal = false;
};

struct ScenePositionData
{
    // Identifier for this position.
    std::string label;
    
    // The target position. If not specified, character won't move, but will still change headings, if specified.
    Vector3 position;
    
    // The rotation, in degrees, for the character to face when they get to the position.
    // It not specified, the character keeps their current heading at the target position.
    float heading = -1.0f;
    
    // If specified, this camera will be switched to when using this position.
    SceneCameraData* camera = nullptr;
	
	// Headings are always defined about the Y-axis.
	Quaternion GetHeadingAsRotation() const { return Quaternion(Vector3::UnitY, heading); }
	Vector3 GetHeadingAsVector() const { return Quaternion(Vector3::UnitY, heading).Rotate(Vector3::UnitZ); }
};

struct SceneRegionOrTriggerData
{
    // A region and trigger only vary in that the label for a
    // trigger is a "noun".
    std::string label;
    
    //TODO: Need a Rect structure.
    float x1, z1, x2, z2;
};

struct SceneActorData
{
    // The model that will represent this actor in the scene.
    Model* model = nullptr;
    
    // The noun associated with this actor, for interactions.
    std::string noun;
    
    // Initial position of the actor in the scene.
    // We'll place the actor here, but this might be overwritten
    // after scene init - maybe due to a SheepScript or something.
    ScenePositionData* position = nullptr;
    
    // Idle gas script file.
    GAS* idleGas = nullptr;
    
    // Talk gas script file.
    GAS* talkGas = nullptr;
    
    // Listen gas script file.
    GAS* listenGas = nullptr;
    
    // An animation played on init. Only first frame will be applied.
    // Overrides any other position value used.
    Animation* initAnim = nullptr;
    
    // If true, this actor is hidden at initialization.
    bool hidden = false;
    
    // If true, this actor will be the EGO (player controlled character).
    bool ego = false;
};

struct SceneModelData
{
    enum class Type
    {
        Scene,		// Model is part of BSP geometry.
        Prop,		// Model is standalone - use "model" for geometry.
        HitTest,	// Model is part of BSP, but shouldn't be rendered. Only used for hit tests.
        GasProp		// Like a prop, but can also be animated by GAS - use "gas" for that.
    };
    
    // Name of the model. For props, this is the name of a MOD file.
    // For scene models, this is the name of the model in BSP.
    std::string name;
    
    // Pretty important - the model to use.
    // Could be null for models in the BSP geometry, which are quite common.
    Model* model = nullptr;
    
    // The type of this model.
	// Dictates whether it is part of the BSP geometry or a separate asset.
    Type type = Type::Scene;
    
    // Noun associated with this object, for interactivity.
    std::string noun;
    
    // Usually, verbs are specified in the NVC file. But it is allowed to
    // specify a verb here if only a single verb response is possible.
    std::string verb;
    
    // An animation played on init. Only first frame will be applied.
    // Only applies to Props. Others will ignore this.
	Animation* initAnim = nullptr;
    
    // For GasProps, the gas file to use.
    GAS* gas = nullptr;
    
    // If true, the model is hidden at creation.
    bool hidden = false;
};

class SIF : public Asset
{
public:
    SIF(std::string name, char* data, int dataLength);
    
    std::string GetSceneModelName() const { return mSceneModelName; }
	
	std::string GetFloorBspModelName() const { return mFloorBspModelName; }
	
	WalkerBoundary* GetWalkerBoundary() const { return mWalkerBoundary; }
	
    Skybox* GetSkybox() const { return mSkybox; }
    
    std::vector<SceneActorData*> GetSceneActorDatas() { return mSceneActorDatas; }
    std::vector<SceneModelData*> GetSceneModelDatas() { return mSceneModelDatas; }
    
	SceneCameraData* GetDefaultRoomCamera() const { return mRoomCameras.size() > 0 ? mRoomCameras[mDefaultRoomCameraIndex] : nullptr; }
	SceneCameraData* GetRoomCamera(std::string cameraName);
	
    ScenePositionData* GetPosition(std::string positionName);
    
    std::vector<Soundtrack*> GetSoundtracks() { return mSoundtracks; }
    
    std::vector<NVC*> GetNounVerbCases() { return mNVCs; }
    
private:
    // Name of the Scene Data asset name that's used in conjunction with this SIF.
    std::string mSceneModelName;
    
    // FLOOR
    // Name of the model in the BSP that is used for character walking.
    std::string mFloorBspModelName;
    
    // BOUNDARIES
	WalkerBoundary* mWalkerBoundary = nullptr;
    
    // CAMERA BOUNDS
    // Name of the MOD file that should be used for camera bounds.
    // Whether the camera bounds are static or not. Defaults to static. (When are they not?)
    std::string mCameraBoundsModelName;
    bool mCameraBoundsDynamic = false;
    
    // GLOBAL LIGHT
    // Global light position, color, ambience.
    Vector3 mGlobalLightPosition = Vector3(100.0f, 100.0f, -500.0f);
    Vector3 mGlobalLightAmbient = Vector3(0.3f, 0.3f, 0.3f);
    
    // SKYBOX
    // The SIF might specify a skybox. But this can also be set from the SCN file instead.
    Skybox* mSkybox = nullptr;
    
    // ACTORS
    std::vector<SceneActorData*> mSceneActorDatas;
    
    // MODELS
    std::vector<SceneModelData*> mSceneModelDatas;
    
    // CAMERAS
    std::vector<SceneCameraData*> mInspectCameras;
    
    std::vector<SceneCameraData*> mRoomCameras;
    int mDefaultRoomCameraIndex = 0;
    
    std::vector<SceneCameraData*> mCinematicCameras;
    std::vector<DialogueSceneCameraData*> mDialogueSceneCameraDatas;
    
    // POSITIONS
    std::vector<ScenePositionData*> mPositions;
    
    // REGIONS & TRIGGERS
    std::vector<SceneRegionOrTriggerData*> mRegions;
    std::vector<SceneRegionOrTriggerData*> mTriggers;
    
    // AMBIENT
    std::vector<Soundtrack*> mSoundtracks;
    
    // ACTIONS
    std::vector<NVC*> mNVCs;
    
    void ParseFromData(char* data, int dataLength);
};
