//
// Mover.h
//
// Clark Kromenaker
//
// Moves an actor to/from designated positions/rotations/scales.
// Mainly used to test out transform hierarchy calculations at runtime.
//
#pragma once
#include "Component.h"

class Mover : public Component
{
	TYPE_DECL_CHILD();
public:
	Mover(Actor* owner);
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	float mDirection = 1.0f;
	float mTimer = 0.0f;
};
