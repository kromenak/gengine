//
// Actor.h
//
// Clark Kromenaker
//
// Any object that exists in the game world and has position/rotation/scale.
//
#pragma once
#include <vector>

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

#include "Component.h"
#include "InputManager.h"
#include "Transform.h"
#include "Services.h"

class Actor
{
public:
	enum class State
	{
		Active,
		Inactive,
		Destroyed
	};
	
	enum class TransformType
	{
		Transform,
		RectTransform
	};
	
    Actor();
	Actor(TransformType transformType);
	
    virtual ~Actor();
    
	void Update(float deltaTime);
    
    template<class T> T* AddComponent();
    template<class T> T* GetComponent();
	
	// STATE
	void SetActive(bool active) { if(mState != State::Destroyed) { mState = active ? State::Active : State::Inactive; } }
	bool IsActive() const;
	
	void Destroy();
	bool IsDestroyed() const { return mState == State::Destroyed; }
	
	void SetIsDestroyOnLoad(bool destroyOnLoad) { mIsDestroyOnLoad = destroyOnLoad; }
	bool IsDestroyOnLoad() const;
	
	// TRANSFORM CONVENIENCE ACCESSORS
	Transform* GetTransform() const { return mTransform; }
	
	Vector3 GetPosition() const { return mTransform->GetPosition(); }
	void SetPosition(Vector3 position) { mTransform->SetPosition(position); }
	
	Quaternion GetRotation() const { return mTransform->GetRotation(); }
	void SetRotation(Quaternion rotation) { mTransform->SetRotation(rotation); }
	
	Vector3 GetScale() const { return mTransform->GetScale(); }
	void SetScale(Vector3 scale) { mTransform->SetScale(scale); }
	
	Vector3 GetForward() const { return mTransform->GetForward(); }
	Vector3 GetRight() const { return mTransform->GetRight(); }
	Vector3 GetUp() const { return mTransform->GetUp(); }
	
protected:
	virtual void OnUpdate(float deltaTime) { }
    
private:
	State mState = State::Active;
	
	// By default, actors are destroyed when a new scene loads.
	bool mIsDestroyOnLoad = true;
	
	// Transform is accessed pretty often, so seems good to cache it.
	Transform* mTransform = nullptr;
	
    // The components that are attached to this actor.
    std::vector<Component*> mComponents;
	
	void AddChild(Actor* child);
	void RemoveChild(Actor* child);
};

template<class T> T* Actor::AddComponent()
{
    T* component = new T(this);
    mComponents.push_back(component);
    return component;
}

template<class T> T* Actor::GetComponent()
{
    for(auto& component : mComponents)
    {
        if(component->IsTypeOf(T::GetType()))
        {
            return static_cast<T*>(component);
        }
    }
    return nullptr;
}
