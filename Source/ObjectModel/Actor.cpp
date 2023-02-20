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

Actor::Actor(const std::string& name) : Actor()
{
    mName = name;
}

Actor::Actor(const std::string& name, TransformType transformType) : Actor(transformType)
{
    mName = name;
}

Actor::~Actor()
{
	//NOTE: GEngine class handles calling "delete". Others should call Destroy if needed.
	
    // Delete all components and clear list.
    for(auto& component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
	if(IsActive() && mUpdateEnabled)
	{
        // Calculate actor's local delta time, based on time scale.
        float localDeltaTime = deltaTime * mTimeScale;
        
		// Do my own update (subclasses can override).
		OnUpdate(localDeltaTime);
		
		// Update all components.
		for(auto& component : mComponents)
		{
            if(component->IsEnabled())
            {
                component->Update(localDeltaTime);
            }
		}
		
        // If enabled, render axes at actor position.
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

            // Enabled components become marked as disabled.
            for(auto& component : mComponents)
            {
                if(component->IsEnabled())
                {
                    component->OnDisable();
                }
            }
		}
		else if(oldState == State::Inactive && mState == State::Active)
		{
			OnActive();

            // Enabled components are back to being enabled.
            for(auto& component : mComponents)
            {
                if(component->IsEnabled())
                {
                    component->OnEnable();
                }
            }
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
