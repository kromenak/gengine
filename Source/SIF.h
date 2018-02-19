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

struct SceneCamera
{
    // The label for this camera.
    // This can be an identifier, or (for inspect camera) the noun/model associated.
    std::string label;
    
    // Camera's angle - I'm guessing this is rotation and pitch.
    Vector2 angle;
    
    // Camera's position.
    Vector3 position;
    
    // Is this the default camera for the room?
    bool isDefault = false;
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
    Vector3* position = nullptr;
    
    // The rotation, in degrees, for the character to face when they get to the position.
    // It not specified, the character keeps their current heading at the target position.
    float* heading = nullptr;
    
    // If specified, this camera will be switched to when using this position.
    SceneCamera* camera = nullptr;
};

struct SceneRegion
{
    std::string label;
    
    //TODO: Need a Rect structure.
    float x1, z1, x2, z2;
};

struct SceneTrigger
{
    std::string noun;
    
    //TODO: Need a Rect structure.
    float x1, z1, x2, z2;
};

class SIF : public Asset
{
public:
    SIF(std::string name, char* data, int dataLength);
    
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
    std::string mCameraBoundsModel;
    bool mCameraBoundsStatic = true;
    
    // GLOBAL LIGHT
    // Global light position, color, ambience.
    Vector3 mGlobalLightPosition = Vector3(100.0f, 100.0f, -500.0f);
    Vector3 mGlobalLightAmbient = Vector3(0.3f, 0.3f, 0.3f);
    
    // SKYBOX
    // Textures for each side. It's OK for some, or all, to be empty.
    std::string mSkyboxLeftTextureName;
    std::string mSkyboxRightTextureName;
    std::string mSkyboxFrontTextureName;
    std::string mSkyboxBackTextureName;
    std::string mSkyboxUpTextureName;
    std::string mSkyboxDownTextureName;
    
    // ACTORS
    std::vector<std::string> mActorNames;
    std::vector<std::string> mActorNouns;
    std::vector<std::string> mActorPositions;
    std::vector<std::string> mActorIdleGasNames;
    std::vector<std::string> mActorTalkGasNames;
    std::vector<std::string> mActorListenGasNames;
    std::vector<bool> mActorHiddenFlags;
    std::vector<bool> mActorEgoFlags;
    
    // MODELS
    std::vector<std::string> mModelNames;
    std::vector<std::string> mModelNouns;
    std::vector<std::string> mModelTypes;
    std::vector<std::string> mModelVerbs;
    std::vector<std::string> mModelInitAnims;
    std::vector<bool> mModelHiddenFlags;
    std::vector<std::string> mModelGasNames;
    
    void ParseFromData(char* data, int dataLength);
};
