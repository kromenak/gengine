//
// Actor.h
//
// Clark Kromenaker
//
// Any object that exists in the game world and
// has position/rotation/scale.
//
#pragma once
#include <vector>

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

#include "Services.h"
#include "InputManager.h"

class Component;

class Actor
{
public:
    Actor();
    ~Actor();
    
    virtual void Update(float deltaTime);
    
    void AddComponent(Component* component);
    
    void Translate(Vector3 offset);
    void Rotate(Vector3 axis, float angle);

    Vector3 GetPosition() const { return mPosition; }
    void SetPosition(Vector3 position);
    
    Quaternion GetRotation() const { return mRotation; }
    void SetRotation(Quaternion rotation);
    
    Vector3 GetScale() const { return mScale; }
    void SetScale(Vector3 scale);
    
    Matrix4 GetWorldTransformMatrix() const { return mWorldTransform; }
    
    Vector3 GetForward() const
    {
        return mRotation.Rotate(Vector3::UnitX);
    }
    
private:
    // A parent actor, if any.
    Actor* mParent = nullptr;
    
    // Position, rotation, and scale of the actor in local space.
    Vector3 mPosition;
    Quaternion mRotation;
    Vector3 mScale;
    
    // Wold transform matrix, which is calculated from position/rotation/scale
    // as well as parent position/rotation/scale.
    Matrix4 mWorldTransform;
    
    // The components that are attached to this actor.
    std::vector<Component*> mComponents;
    
    void UpdateWorldTransform();
};
