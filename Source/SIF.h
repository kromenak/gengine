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
#include "Heading.h"
#include "Quaternion.h"
#include "Rect.h"
#include "Vector2.h"
#include "Vector3.h"

class Animation;
class GAS;
class Model;
class NVC;
class SheepScript;
class Skybox;
class Soundtrack;
class Texture;
class WalkerBoundary;

struct SceneCamera
{
    // The label for this camera.
    // This can be an identifier, or (for an inspect camera) the noun/model associated.
    std::string label;
    
    // Camera's angle - yaw and pitch.
    Vector2 angle;
    
    // Camera's position.
    Vector3 position;
};

struct RoomSceneCamera : public SceneCamera
{
	// If true, this is the 'default" room camera.
	// Default room camera is used if one is needed, but none is specified.
	bool isDefault = false;
};

struct DialogueSceneCamera : public SceneCamera
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

struct ScenePosition
{
    // Identifier for this position.
    std::string label;
    
    // The target position. If not specified, character won't move, but will still change headings, if specified.
    Vector3 position;
    
    // The heading to use at the target position.
    // It not specified, the character keeps their current heading at the target position.
	Heading heading = Heading::None;
    
    // If specified, this camera will be switched to when using this position.
	std::string cameraName;
    SceneCamera* camera = nullptr;
};

struct SceneRegionOrTrigger
{
    // A region and trigger only vary in that the label for a
    // trigger is a "noun".
    std::string label;
    
    // The rectangular area. Note that this is on X/Z plane, despite Rect var names.
	Rect rect;
};

struct SceneActor
{
    // The model that will represent this actor in the scene.
    Model* model = nullptr; //TODO: Should we just store the name until Scene load?
    
    // The noun associated with this actor, for interactions.
    std::string noun;
    
    // Initial position of the actor in the scene.
    // We'll place the actor here, but this might be overwritten
    // after scene init - maybe due to a SheepScript or something.
	std::string positionName;
	ScenePosition* position = nullptr;
    
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

struct SceneModel
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
	~SIF();
    
    const std::string& GetSceneAssetName() const { return mSceneModelName; }
	
	const std::string& GetFloorModelName() const { return mFloorModelName; }
	
	WalkerBoundary* GetWalkerBoundary() const { return mWalkerBoundary; }
    Skybox* GetSkybox() const { return mSkybox; }
    
    const std::vector<SceneActor*>& GetSceneActors() { return mSceneActors; }
    const std::vector<SceneModel*>& GetSceneModels() { return mSceneModels; }
    
	const SceneCamera* GetDefaultRoomCamera() const { return mRoomCameras.size() > 0 ? mRoomCameras[mDefaultRoomCameraIndex] : nullptr; }
	const SceneCamera* GetRoomCamera(const std::string& cameraName) const;
	
	const ScenePosition* GetPosition(const std::string& positionName) const;
    
    const std::vector<Soundtrack*>& GetSoundtracks() { return mSoundtracks; }
    
    const std::vector<NVC*>& GetActionSets() { return mNVCs; }
    
private:
    // Name of the Scene Data asset name that's used in conjunction with this SIF.
    std::string mSceneModelName;
    
    // FLOOR
    // Name of the model in the BSP that is used for character walking.
    std::string mFloorModelName;
    
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
    std::vector<SceneActor*> mSceneActors;
    
    // MODELS
    std::vector<SceneModel*> mSceneModels;
    
    // CAMERAS
    std::vector<SceneCamera*> mInspectCameras;
    
    std::vector<SceneCamera*> mRoomCameras;
    int mDefaultRoomCameraIndex = 0;
    
    std::vector<SceneCamera*> mCinematicCameras;
    std::vector<DialogueSceneCamera*> mDialogueSceneCameras;
    
    // POSITIONS
    std::vector<ScenePosition*> mPositions;
    
    // REGIONS & TRIGGERS
    std::vector<SceneRegionOrTrigger*> mRegions;
    std::vector<SceneRegionOrTrigger*> mTriggers;
    
    // AMBIENT
    std::vector<Soundtrack*> mSoundtracks;
    
    // ACTIONS
    std::vector<NVC*> mNVCs;
    
    void ParseFromData(char* data, int dataLength);
};
