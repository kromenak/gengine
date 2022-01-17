//
// Clark Kromenaker
//
// Camera used to actually play the game. Obeys all game world laws.
//
// Because GK3 is technically a "3rd person" game and the player directly controls the camera,
// you can probably consider this to be the "player actor" or "player controller". So, most user inputs are checked here.
//
#pragma once
#include "Actor.h"

class GKObject;
class Model;
class OptionBar;

class GameCamera : public Actor
{
public:
    GameCamera();
	
	void SetBounds(Model* boundsModel) { mBoundsModel = boundsModel; }
	void SetBoundsEnabled(bool enabled) { mBoundsEnabled = enabled; }
	
	void SetAngle(const Vector2& angle);
	void SetAngle(float yaw, float pitch);
	
	Camera* GetCamera() { return mCamera; }
    
    void SetSceneActive(bool active) { mSceneActive = active; }
	
protected:
    void OnUpdate(float deltaTime) override;
    
private:
	// Camera movement speed in units/second.
	// A modifier key (alt) causes the camera to move even faster.
	// These values were derived from trial and error!
	const float kSpeed = 500.0f;
	const float kFastSpeedMultiplier = 2.0f;
	
	// Camera rotation speed in radians/second. Again, trial and error!
	const float kRotationSpeed = Math::kPi / 2.0f;
	
	// Mouse range in pixels to move as fast as keyboard keys.
	// Smaller values mean mouse movement is more sensitive/speedier.
	const float kMouseRangePixels = 50.0f;
	
	// Reference to underlying camera component.
    Camera* mCamera = nullptr;
    
    // Option bar - used to change settings, quit game, etc.
    OptionBar* mOptionBar = nullptr;
	
	// Height of camera above ground. Always try to maintain some height above ground.
	// So, if moving down a hill, the camera follows the slope, for example.
	// Default value is derived from trial and error!
	const float kDefaultHeight = 60.0f;
	float mHeight = kDefaultHeight;
	
	// A model whose triangles are used as collision for the camera.
	Model* mBoundsModel = nullptr;
		
	// If true, camera bounds are turned on. If false, they are disabled.
    bool mBoundsEnabled = true;
	
	// The last object hovered over. Used for toggling cursor highlight color.
	std::string mLastHoveredNoun;
    
    // If true, scene is "active", so perform scene updates (e.g. camera movement).
    // When scene is not active, game camera still handles some user input stuff.
    bool mSceneActive = true;
	
    void SceneUpdate(float deltaTime);
    
	void ResolveCollisions(Vector3& newPosition, const Vector3& originalPosition);
};
