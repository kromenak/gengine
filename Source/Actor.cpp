//
// Actor.cpp
//
// Clark Kromenaker
//
#include "Actor.h"

#include "Debug.h"
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
	//NOTE: GEngine class handles calling "delete". Others should call Destroy if needed.
	
	// Rather than have actors remove themselves, GEngine now does this during deletion.
	// It seems much safer for GEngine to coordinate all this, rather than any actor being able to remove itself anytime.
    //GEngine::RemoveActor(this);
	
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
	OnUpdate(deltaTime);
	
    // Update all components.
    for(auto& component : mComponents)
    {
        component->Update(deltaTime);
    }
	
	if(Debug::RenderActorTransformAxes())
	{
		Debug::DrawAxes(mTransform->GetLocalToWorldMatrix());
	}
	//Debug::DrawLine(GetPosition(), GetPosition() + GetForward() * 5.0f, Color32::Red);
}

void Actor::Destroy()
{
	mState = State::Dead;
	
	// Destroying an actor also destroys its children.
	for(auto& child : mTransform->GetChildren())
	{
		child->GetOwner()->Destroy();
	}
}

bool Actor::IsDestroyOnLoad() const
{
	// For children, the parent decides "destroy on load" functionality.
	if(mTransform->GetParent() != nullptr)
	{
		return mTransform->GetParent()->GetOwner()->IsDestroyOnLoad();
	}
	return mIsDestroyOnLoad;
}
