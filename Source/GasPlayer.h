//
// GasPlayer.cpp
//
// Clark Kromenaker
//
// A component that can play a GAS file.
//
#pragma once
#include "Component.h"

class GAS;
class AnimationPlayer;

class GasPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    GasPlayer(Actor* owner);
    
    void Update(float deltaTime) override;
    
    void SetGas(GAS* gas) { mGas = gas; mNodeIndex = 0; }
    
    void SetAnimationPlayer(AnimationPlayer* animationPlayer) { mAnimationPlayer = animationPlayer; }
    AnimationPlayer* GetAnimationPlayer() { return mAnimationPlayer; }
    
private:
    // Animation player.
    AnimationPlayer* mAnimationPlayer = nullptr;
    
    // The GAS asset being played.
    GAS* mGas = nullptr;
    
    // The current node index being played.
    int mNodeIndex = 0;
    
    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;
    
    void ProcessNextNode();
};

