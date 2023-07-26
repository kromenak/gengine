//
// Clark Kromenaker
//
// Any object that exists in the game world and has position/rotation/scale.
//
#pragma once
#include <vector>

#include "Component.h"
#include "InputManager.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Vector3.h"

// As a rule, all all Actors must have a Transform.
// This allows specifying which type to construct with.
enum class TransformType
{
    Transform,
    RectTransform
};

class Actor
{
public:
	enum class State
	{
		Active,
		Inactive,
		Destroyed
	};
	
    Actor();
	Actor(TransformType transformType);
    Actor(const std::string& name);
    Actor(const std::string& name, TransformType transformType);
    virtual ~Actor();
    
	void Update(float deltaTime);
    
    template<class T> T* AddComponent();
    template<class T, class... Args> T* AddComponent(Args&&... args);
    template<class T> T* GetComponent();
    template<class T> T* GetComponentInParents();
    
    const std::string& GetName() const { return mName; }
    void SetName(const std::string& name) { mName = name; }
	
	// STATE
	void SetActive(bool active);
	bool IsActive() const;
	
	void Destroy();
	bool IsDestroyed() const { return mState == State::Destroyed; }
	
	void SetIsDestroyOnLoad(bool destroyOnLoad) { mIsDestroyOnLoad = destroyOnLoad; }
	bool IsDestroyOnLoad() const;
    
    void SetTimeScale(float timeScale) { mTimeScale = timeScale; }
    void SetUpdateEnabled(bool updateEnabled) { mUpdateEnabled = updateEnabled; }
	
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
	
	Vector3 GetWorldPosition() const { return mTransform->GetWorldPosition(); }
	void SetWorldPosition(const Vector3& position) { mTransform->SetWorldPosition(position); }
	
	Quaternion GetWorldRotation() const { return mTransform->GetWorldRotation(); }
	void SetWorldRotation(const Quaternion& rotation) { mTransform->SetWorldRotation(rotation); }
	
	Vector3 GetWorldScale() const { return mTransform->GetWorldScale(); }
	
protected:
	virtual void OnActive() { }
	virtual void OnInactive() { }
	
	virtual void OnUpdate(float deltaTime) { }
    
private:
    // The actor's name - meant for debugging purposes.
    std::string mName;
    
    // State of the actor - used for lifetime management and whether the actor receives updates.
	State mState = State::Active;
	
	// By default, actors are destroyed when a new scene loads.
	bool mIsDestroyOnLoad = true;
	
    // This actor's time scale.
    // Can use to make actor update faster, slower.
    float mTimeScale = 1.0f;
    
    // Is update enabled for this actor?
    // Differs from time scale because timescale of 0.0 still calls update! This disables it entirely.
    bool mUpdateEnabled = true;
    
	// Transform is accessed pretty often, so seems good to cache it.
	Transform* mTransform = nullptr;
	
    // The components that are attached to this actor.
    std::vector<Component*> mComponents;
};

template<class T> T* Actor::AddComponent()
{
    T* component = new T(this);
    mComponents.push_back(component);
    return component;
}

template<class T, class... Args> T* Actor::AddComponent(Args&&... args)
{
    // This constructor allows passing arbitrary constructor arguments when adding a component.
    // This is an example of a "parameter pack" and "perfect forwarding" in C++.
    // The "..." define a parameter pack, which is an arbitrary list of zero or more arguments.
    // Passing args as "Args&&" and using std::forward enables perfect forwarding (an efficiency thing, cause why not).
    T* component = new T(this, std::forward<Args>(args)...);
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

template<class T> T* Actor::GetComponentInParents()
{
    // If I've got the component, just return that!
    T* myT = GetComponent<T>();
    if(myT != nullptr) { return myT; }

    // Otherwise, search in parents.
    Transform* parent = mTransform->GetParent();
    if(parent != nullptr)
    {
        return parent->GetOwner()->GetComponentInParents<T>();
    }

    // If I have no parent, and I don't have it, return null.
    return nullptr;
}