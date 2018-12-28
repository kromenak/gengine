//
// Walker.h
//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of
// moving an actor's legs and walking from point to point in the game world.
//
#pragma once
#include "Component.h"

#include <string>

class Animation;

class Walker : public Component
{
	TYPE_DECL_CHILD();
public:
	Walker(Actor* owner);
	
	
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	Animation* mWalkStartAnim = nullptr;
	Animation* mWalkLoopAnim = nullptr;
	Animation* mWalkStopAnim = nullptr;

};
