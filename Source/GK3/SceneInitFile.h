//
// Clark Kromenaker
//
// Scene Initialization Files (SIFs for short) define the layout and structure of a scene.
// To load a scene's actors/models/audio/logic, at least one SIF file must be specified.
//
// A scene can have one or more SIF files. Each scene must at least specify a "General" SIF file.
// SIF files for specific timeblocks can also be included.
//
// On disk, these assets have a SIF extension.
//
#pragma once
#include "Asset.h"

#include <vector>

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

struct GeneralBlock
{
    // A condition that must be true for this block to be active.
    // Condition usually relates to current timeblock or some game progress flags.
	std::string conditionText;
	SheepScript* condition = nullptr;
	
	std::string sceneAssetName;

	std::string floorModelName;
	std::string walkerBoundaryTextureName;
	Vector2 walkerBoundarySize;
	Vector2 walkerBoundaryOffset;

    std::vector<std::string> cameraBoundsModelNames;
	bool cameraBoundsDynamic = false;
	
	Vector3 globalLightPosition;
    //globalLightColor?
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
    std::string setName; //TODO: this seems to never be used...maybe abandoned
    
    // If true, this camera appears in the toolbar when the conversation is active.
    bool showInToolbar = false; //TODO: also seems to never be used!
    
    // If true, camera is used when the conversation ends.
    bool isInitial = false;
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
};

struct SceneRegionOrTrigger
{
    // A region and trigger only vary in that the label for a trigger is a "noun".
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

struct SceneConversation
{
    // Name of the conversation these settings relate to.
    std::string name;

    // Name of the actor these settings relate to.
    // Each actor involved in the conversation has its own struct instance.
    std::string actorName;

    // Custom talk/listen autoscripts for this conversation.
    // Only need to be specified if these differ from the actor's defaults.
    GAS* talkGas = nullptr;
    GAS* listenGas = nullptr;

    // Animations to play when entering or exiting the conversation.
    Animation* enterAnim = nullptr;
    Animation* exitAnim = nullptr;
};

template<typename T>
struct ConditionalBlock
{
    // The condition under which these items should be used - SheepScript evaluates to true/false.
	std::string conditionText;
	SheepScript* condition = nullptr;

    // The items to use under this condition.
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

    const std::vector<ConditionalBlock<SceneConversation>>& GetConversationBlocks() const { return mConversations; }
	
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

    // LISTENERS (aka CONVERSATIONS)
    std::vector<ConditionalBlock<SceneConversation>> mConversations;
	
	// ACTIONS
	// This one's also pointers b/c NVCs are Assets.
    std::vector<ConditionalBlock<NVC*>> mActions;
	
	void ParseFromData(char* data, int dataLength);
};
