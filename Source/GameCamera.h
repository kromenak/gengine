//
// GameCamera.h
//
// Clark Kromenaker
//
// Camera used to actually play the game. Obeys all game world laws.
//
#pragma once
#include "Actor.h"

class GameCamera : public Actor
{
public:
    GameCamera();
	
	void SetAngle(const Vector2& angle);
	void SetAngle(float yaw, float pitch);
	
protected:
    void OnUpdate(float deltaTime) override;
    
private:
    Camera* mCamera = nullptr;
};
