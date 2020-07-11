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
    GEngine::Instance()->AddActor(this);
	
	// Add transform component.
	mTransform = AddComponent<Transform>();
}

Actor::Actor(TransformType transformType)
{
	GEngine::Instance()->AddActor(this);
	
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
	if(mState == State::Active)
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
	}
}

void Actor::SetActive(bool active)
{
	// Don't allow setting active/inactive if destroyed.
	if(mState != State::Destroyed)
	{
		// Save old state.
		Actor::State oldState = mState;
		
		// Update state.
		mState = active ? State::Active : State::Inactive;
		
		// Handle transitions from active to inactive and vice-versa.
		if(oldState == State::Active && mState == State::Inactive)
		{
			OnInactive();
		}
		else if(oldState == State::Inactive && mState == State::Active)
		{
			OnActive();
		}
	}
}

bool Actor::IsActive() const
{
	// Easy enough...
	if(mState != State::Active) { return false; }
	
	// If we're active, our parent might not be active, which means we aren't active.
	Transform* parent = mTransform->GetParent();
	if(parent != nullptr)
	{
		return parent->GetOwner()->IsActive();
	}
	
	// We're active!
	return true;
}

void Actor::Destroy()
{
	mState = State::Destroyed;
	
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
