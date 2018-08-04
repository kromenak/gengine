//
// Actor.cpp
//
// Clark Kromenaker
//
#include "Actor.h"
#include "Component.h"
#include "GEngine.h"

Actor::Actor() : mPosition(0, 0, 0),
    mRotation(0, 0, 0, 1),
    mScale(1, 1, 1)
{
    GEngine::AddActor(this);
    UpdateWorldTransform();
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
    // Update all components.
    for(auto& component : mComponents)
    {
        component->Update(deltaTime);
    }
}

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
            return *static_cast<T*>(component);
        }
    }
    return nullptr;
}

void Actor::Translate(Vector3 offset)
{
    SetPosition(mPosition + offset);
}

void Actor::Rotate(Vector3 axis, float angle)
{
    SetRotation(Quaternion(axis, angle) * GetRotation());
}

void Actor::SetPosition(Vector3 position)
{
    mPosition = position;
    UpdateWorldTransform();
}

void Actor::SetRotation(Quaternion rotation)
{
    mRotation = rotation;
    UpdateWorldTransform();
}

void Actor::SetScale(Vector3 scale)
{
    mScale = scale;
    UpdateWorldTransform();
}

void Actor::UpdateWorldTransform()
{
    // Get translate/rotate/scale matrices.
    Matrix4 translateMatrix = Matrix4::MakeTranslate(mPosition);
    Matrix4 rotateMatrix = Matrix4::MakeRotate(mRotation);
    Matrix4 scaleMatrix = Matrix4::MakeScale(mScale);
    mWorldTransform = translateMatrix * rotateMatrix * scaleMatrix;
}
