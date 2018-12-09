//
// Scene.h
//
// Clark Kromenaker
//
// From GK3 docs: "a stage is everything associated with a specific game location
// during a specific timeblock (e.g. Dining Room, Day 1, 10am).
//
#pragma once
#include <string>

class ActionBar;
class Actor;
class BSP;
class GameCamera;
class Ray;
class SceneData;
class SIF;
class Skybox;
class Vector3;

class Scene
{
public:
    Scene(std::string name, std::string timeCode);
    
    void InitEgoPosition(std::string positionName);
    
    void Interact(const Ray& ray);
	
	float GetFloorY(const Vector3& position);
    
private:
    // The scene name, both general and specific.
    // General name is time-agnostice (Ex: DIN).
    // Specific name includes the day/time (Ex: DIN110A).
    std::string mGeneralName;
    std::string mSpecificName;
    
    // Scene initialization files for the scene (general) and specific timeblock (specific).
    SIF* mGeneralSIF = nullptr;
    SIF* mSpecificSIF = nullptr;
    
    // SceneData (SCN) derived from SIFs. Mostly tells us which BSP to use.
    SceneData* mSceneData = nullptr;
    
    // The BSP we'll use to render this scene's geometry.
    BSP* mSceneBSP = nullptr;
    
    // The skybox this scene uses. Taken from either the SIF or SCN files.
    Skybox* mSkybox = nullptr;
    
    // The game camera used to move around.
    GameCamera* mCamera = nullptr;
    
    // The actor who we are controlling in the scene.
    Actor* mEgo = nullptr;
	
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
