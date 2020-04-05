//
// SphereCollider.h
//
// Clark Kromenaker
//
// A component that detects and handles collisions using a sphere shape.
//
#pragma once
#include "Component.h"

class SphereCollider : public Component
{
	TYPE_DECL_CHILD();
public:
	SphereCollider(Actor* owner);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
};
