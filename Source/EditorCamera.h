//
// EditorCamera.h
//
// Clark Kromenaker
//
// A camera that can fly all over the scene and doesn't have collision (noclip!)
//
#pragma once
#include "Actor.h"

class EditorCamera : public Actor
{
public:
    EditorCamera();
    void Update(float deltaTime) override;
};
