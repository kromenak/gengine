//
// Component.h
//
// Clark Kromenaker
//
// A component is a reusable bit of functionality
// that can be attached to an Actor.
//
#pragma once
#include "Actor.h"

class Component
{
public:
    Component(Actor* owner);
    
    virtual void Update(float deltaTime) { }
    
    Actor* GetOwner() { return mOwner; }
    
protected:
    Actor* mOwner;
};
