//
// Component.h
//
// Clark Kromenaker
//
// A component is a reusable bit of functionality
// that can be attached to an Actor.
//

#pragma once

class Actor;

class Component
{
public:
    Component(Actor* owner);
    
    virtual void Update(float deltaTime) = 0;
    
protected:
    Actor* mOwner;
};
