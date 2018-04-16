//
// SIF.h
//
// Clark Kromenaker
//
// A (S)cene (I)nitialization (F)ile, which is an asset used to load
// and initialize a scene (or room) in the game.
//
// If you wanted to load a particular scene in the game, you'd need to
// pass a SIF asset to kick the loading process off!
//
// In-memory representation of .SIF files.
//
#pragma once
#include "Asset.h"
#include "Vector2.h"
#include "Vector3.h"
#include <vector>

class Texture;
class Model;
class NVC;
class Soundtrack;

struct SceneCamera
{
    // The label for this camera.
    // This can be an identifier, or (for inspect camera) the noun/model associated.
    std::string label;
    
    // Camera's angle - yaw and pitch.
    Vector2 angle;
    
    // Camera's position.
    Vector3 position;
};

struct DialogueCamera : public SceneCamera
{
    // Specifies name of dialogue for which this camera is used.
    std::string dialogueName;
    
    // Specifies name of set this camera belongs to.
    // Allows logic to specify a set name, which randomly picks a camera in that set.
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
    Vector3 position ;
    
    // The rotation, in degrees, for the character to face when they get to the position.
    // It not specified, the character keeps their current heading at the target position.
    float heading = -1.0f;
    
    // If specified, this camera will be switched to when using this position.
    SceneCamera* camera = nullptr;
};

struct SceneRegionOrTrigger
{
    // A region and trigger only vary in that the label for a
    // trigger is a "noun".
    std::string label;
    
    //TODO: Need a Rect structure.
    float x1, z1, x2, z2;
};

struct SceneSkybox
{
    Texture* leftTexture = nullptr;
    Texture* rightTexture = nullptr;
    Texture* frontTexture = nullptr;
    Texture* backTexture = nullptr;
    Texture* upTexture = nullptr;
    Texture* downTexture = nullptr;
};

struct ActorDefinition
{
    // The model that will represent this actor in the scene.
    Model* model = nullptr;
    
    // The noun associated with this actor, for interactions.
    std::string noun;
    
    // Initial position of the actor in the scene.
    // We'll place the actor here, but this might be overwritten
    // after scene init - maybe due to a Sheep script or something.
    ScenePosition* position = nullptr;
    
    // IDLE GAS FILE
    // TALK GAS FILE
    // LISTEN GAS FILE
    
    // If true, this actor is hidden at initialization.
    bool hidden = false;
    
    // If true, this actor will be the EGO (player controlled character).
    bool ego = false;
};

struct ModelDefinition
{
    enum class Type
    {
        Scene,
        Prop,
        HitTest,
        GasProp
    };
    
    // Name of the model. For props, this is the name of a MOD file.
    // For scene models, this is the name of the model in BSP.
    std::string name;
    
    // Pretty important - the model to use.
    // Could be null for models in the BSP geometry, which are quite common.
    Model* model = nullptr;
    
    // The type of this model. Dictates whether it is part of the BSP geometry or a separate asset.
    Type type = Type::Scene;
    
    // Noun associated with this object, for interactivity.
    std::string noun;
    
    // Usually, verbs are specified in the NVC file. But it is allowed to
    // specify a verb here if only a single verb response is possible.
    std::string verb;
    
    // First frame of this anim will be applies on initialization.
    // Only applies to Props. Others will ignore this.
    //TODO: InitAnim
    
    // For GasProps, the gas file to use.
    //TODO: GasFile
    
    // If true, the model is hidden at initialization.
    bool hidden = false;
};

class SIF : public Asset
{
public:
    SIF(std::string name, char* data, int dataLength);
    
    std::string GetSCNName() { return mSceneAssetName; }
    
    std::vector<ActorDefinition*> GetActorDefinitions() { return mActorDefinitions; }
    
    SceneCamera* GetDefaultRoomCamera() { return mRoomCameras[mDefaultRoomCameraIndex]; }
    
    std::vector<Soundtrack*> GetSoundtracks() { return mSoundtracks; }
    
private:
    // Name of the Scene asset that is used in conjunction with this SIF.
    std::string mSceneAssetName;
    
    // FLOOR
    // Name of the model in the BSP scene that is used for character walking.
    std::string mFloorSceneModelName;
    
    // BOUNDARIES
    std::string mWalkBoundaryTextureName;
    Vector2 mWalkBoundarySize;
    Vector2 mWalkBoundaryOffset;
    
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
    SceneSkybox mSkybox;
    
    // ACTORS
    std::vector<ActorDefinition*> mActorDefinitions;
    
    // MODELS
    std::vector<ModelDefinition*> mModelDefinitions;
    
    // CAMERAS
    std::vector<SceneCamera*> mInspectCameras;
    
    std::vector<SceneCamera*> mRoomCameras;
    int mDefaultRoomCameraIndex = 0;
    
    std::vector<SceneCamera*> mCinematicCameras;
    std::vector<DialogueCamera*> mDialogueCameras;
    
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
