//
// Mover.h
//
// Clark Kromenaker
//
// Description
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
