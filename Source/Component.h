//
// Component.h
//
// Clark Kromenaker
//
// A component is a reusable bit of functionality
// that can be attached to an Actor.
//
#pragma once
#include "Type.h"

class Actor;

class Component
{
    TYPE_DECL_BASE();
public:
    Component(Actor* owner);
    
    virtual void Update(float deltaTime) { }
    
    Actor* GetOwner() { return mOwner; }
    
protected:
    Actor* mOwner;
};
