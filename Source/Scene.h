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

class ActionBar;
class AnimationPlayer;
class BSP;
class GameCamera;
class GKActor;
class Ray;
class SceneModel;
class SIF;
class Skybox;
class Vector3;

class Scene
{
public:
    Scene(std::string name, std::string timeCode);
	~Scene();
	
	void OnSceneEnter();
	
    void InitEgoPosition(std::string positionName);
	void SetCameraPosition(std::string cameraName);
	
	bool CheckInteract(const Ray& ray);
    void Interact(const Ray& ray);
	
	float GetFloorY(const Vector3& position);
	
	GKActor* GetEgo() const { return mEgo; }
	GKActor* GetActorByModelName(std::string modelName);
	GKActor* GetActorByNoun(std::string noun);
	
	void ApplyTextureToSceneModel(std::string modelName, Texture* texture);
	
	AnimationPlayer* GetAnimationPlayer() { return mAnimationPlayer; }
    
private:
    // The scene name, both general and specific.
    // General name is time-agnostic (Ex: DIN).
    // Specific name includes the day/time (Ex: DIN110A).
    std::string mGeneralName;
    std::string mSpecificName;
	
	// Contains scene data references for the current location/timeblock.
	SceneData mSceneData;
	
	// The animation player for the scene.
	AnimationPlayer* mAnimationPlayer = nullptr;
	
    // The game camera used to move around.
    GameCamera* mCamera = nullptr;
	
	// GKActors created for this scene.
	std::vector<GKActor*> mActors;
	
    // The actor who we are controlling in the scene.
    GKActor* mEgo = nullptr;
	
	// Action bar, which the player uses to perform actions on scene objects.
	ActionBar* mActionBar = nullptr;
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
