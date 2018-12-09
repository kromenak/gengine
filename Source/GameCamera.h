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
	
protected:
    void UpdateInternal(float deltaTime) override;
    
private:
    Camera* mCamera = nullptr;
};
