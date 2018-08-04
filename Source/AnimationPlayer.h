//
// AnimationPlayer.cpp
//
// Clark Kromenaker
//
// Plays animations!
//
#pragma once
#include "Component.h"

class Animation;

class AnimationPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    AnimationPlayer(Actor* owner);
    
    void Update(float deltaTime) override;
    
    void Play(Animation* animation);
    
private:
    Animation* mAnimation = nullptr;
};
