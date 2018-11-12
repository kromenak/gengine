//
// Actor.cpp
//
// Clark Kromenaker
//
#include "Actor.h"

#include "Component.h"
#include "GEngine.h"
#include "RectTransform.h"

Actor::Actor()
{
	GEngine::AddActor(this);
	
	// Add transform component.
	mTransform = AddComponent<Transform>();
}

Actor::Actor(TransformType transformType)
{
	GEngine::AddActor(this);
	
	// Add transform component.
	if(transformType == TransformType::Transform)
	{
		mTransform = AddComponent<Transform>();
	}
	else
	{
		mTransform = AddComponent<RectTransform>();
	}
}

Actor::~Actor()
{
    GEngine::RemoveActor(this);
    
    // Delete all components and clear list.
    for(auto& component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
	// Do my own update (subclasses can override).
	UpdateInternal(deltaTime);
	
    // Update all components.
    for(auto& component : mComponents)
    {
        component->Update(deltaTime);
    }
}
