//
// Actor.cpp
//
// Clark Kromenaker
//
#include "Actor.h"
#include "Component.h"
#include "GEngine.h"

Actor::Actor() : mPosition(Vector3(0, 0, 0)),
    mRotation(0, 0, 0, 1),
    mScale(Vector3(1, 1, 1))
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

void Actor::AddComponent(Component* component)
{
    // Add to vector, but only if not already added.
    auto it = std::find(mComponents.begin(), mComponents.end(), component);
    if(it == mComponents.end())
    {
        mComponents.push_back(component);
    }
}

void Actor::Translate(Vector3 offset)
{
    SetPosition(mPosition + offset);
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
    //mWorldTransform = scaleMatrix * rotateMatrix * translateMatrix;
    //std::cout << mWorldTransform << std::endl;
}
