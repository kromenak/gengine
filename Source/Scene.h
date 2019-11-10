//
// Scene.h
//
// Clark Kromenaker
//
// Encapsulates all the loaded actors and components for a single conceptual
// location or place in the game. A scene can represent a single level, a set
// of menus, or a particular location in a game.
//
// For GK3, a "scene" is a particular location in the game world (Lobby, Dining Room,
// Room 25, Train Station, etc). The game can only be "in" one scene at a time.
//
// From GK3 docs: "a stage is everything associated with a specific game location
// during a specific timeblock (e.g. Dining Room, Day 1, 10am).
//
#pragma once
#include <string>
#include <vector>

#include "SceneData.h"
#include "Timeblock.h"

class ActionBar;
class AnimationPlayer;
class BSP;
class GameCamera;
class GKActor;
class Ray;
class SceneModel;
class SIF;
class Skybox;
class SoundtrackPlayer;
class Vector3;

class Scene
{
public:
    Scene(std::string name, std::string timeblock);
	~Scene();
	
	void Load();
	void Unload();
	
    void InitEgoPosition(const std::string& positionName);
	void SetCameraPosition(const std::string& cameraName);
	
	bool CheckInteract(const Ray& ray) const;
    void Interact(const Ray& ray);
	
	float GetFloorY(const Vector3& position) const;
	
	const std::string& GetEgoName() const { return mEgoName; }
	GKActor* GetEgo() const { return mEgo; }
	
	GKActor* GetActorByModelName(const std::string& modelName) const;
	GKActor* GetActorByNoun(const std::string& noun) const;
	
	const ScenePosition* GetPosition(const std::string& positionName) const;
	
	void ApplyTextureToSceneModel(const std::string& modelName, Texture* texture);
	void SetSceneModelVisibility(const std::string& modelName, bool visible);
	bool IsSceneModelVisible(const std::string& modelName) const;
	bool DoesSceneModelExist(const std::string& modelName) const;
	
	AnimationPlayer* GetAnimationPlayer() const { return mAnimationPlayer; }
	SoundtrackPlayer* GetSoundtrackPlayer() const { return mSoundtrackPlayer; }
    
private:
	// Location is 3-letter code (e.g. DIN).
	// Timeblock is day/time code (e.g. 110A).
	std::string mLocation;
	Timeblock mTimeblock;
	
	// Contains scene data references for the current location/timeblock.
	// If not null, means we're loaded!
	SceneData* mSceneData = nullptr;
	
	// The animation player for the scene.
	AnimationPlayer* mAnimationPlayer = nullptr;
	
	// The soundtrack player for the scene.
	SoundtrackPlayer* mSoundtrackPlayer = nullptr;
	
    // The game camera used to move around.
    GameCamera* mCamera = nullptr;
	
	// GKActors created for this scene.
	std::vector<GKActor*> mActors;
	
    // The name of actor and actor who we are controlling in the scene.
	// We sometimes need just the name - that's safer during scene loading.
	std::string mEgoName;
    GKActor* mEgo = nullptr;
	
	// Action bar, which the player uses to perform actions on scene objects.
	ActionBar* mActionBar = nullptr;
	
	void ExecuteAction(const Action* action);
};

/*
What do we need to do to load a stage?
 1) Parse general SIF asset for the stage.
 2) Parse specific SIF asset for the current timeblock.
 3) Parse SCN asset specified in SIF.
 4) Parse BSP asset specified in SCN.
 5) Set BSP to be rendered.
 6) Load and use boundary map for walking
 7) Load and use camera bounds for the camera movement.
 8) Set global light values.
 9) Load and render skybox.
 10) Load, place, and initialize any actors in the stage.
 11) Load, place, and initialize any models in the stage.
 12) Position camera based on "default" ROOM_CAMERAS entry (or first item).
 13) Load and parse NVC files for current day.
*/
