//
// Component.h
//
// Clark Kromenaker
//
// A component is a reusable bit of functionality
// that can be attached to an Actor.
//
#pragma once
#include "Type.h" // For homebrew RTTI.

class Actor;

class Component
{
    TYPE_DECL_BASE();
public:
    Component(Actor* owner);
	virtual ~Component() { }
    
	void Update(float deltaTime) { UpdateInternal(deltaTime); }
    
    Actor* GetOwner() const { return mOwner; }
    
protected:
	Actor* mOwner = nullptr;
	
	virtual void UpdateInternal(float deltaTime) { }
};
