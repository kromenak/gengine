//
// AudioListener.h
//
// Clark Kromenaker
//
// Represents the location of the "ears" in the scene.
// Often attached to the camera...but sometimes not.
//
#pragma once
#include "Component.h"

class AudioListener : public Component
{
public:
    AudioListener(Actor* owner);
    
    void Update(float deltaTime) override;
};
