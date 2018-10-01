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
	
	// Remove from parent.
	SetParent(nullptr);
}

void Actor::Update(float deltaTime)
{
    // Update all components.
    for(auto& component : mComponents)
    {
        component->Update(deltaTime);
    }
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

void Actor::SetParent(Actor* parent)
{
	// Remove from existing parent.
	if(mParent != nullptr)
	{
		mParent->RemoveChild(this);
		mParent = nullptr;
	}
	
	//TODO: Ensure not setting as parent one of my children.
	
	// Attach to new parent.
	mParent = parent;
	if(mParent != nullptr)
	{
		mParent->AddChild(this);
	}
	
	// Force world transform update.
	UpdateWorldTransform();
}

void Actor::UpdateWorldTransform()
{
    // Get translate/rotate/scale matrices.
    Matrix4 translateMatrix = Matrix4::MakeTranslate(mPosition);
    Matrix4 rotateMatrix = Matrix4::MakeRotate(mRotation);
    Matrix4 scaleMatrix = Matrix4::MakeScale(mScale);
	
	// Combine in order (Scale, Rotate, Translate) to generate world transform matrix.
    mWorldTransform = translateMatrix * rotateMatrix * scaleMatrix;
	
	// If I'm a child, multiply parent transform into the mix.
	if(mParent != nullptr)
	{
		mWorldTransform = mWorldTransform * mParent->GetWorldTransformMatrix();
	}
	
	// Tell my children to update their world transforms too.
	for(auto& child : mChildren)
	{
		child->UpdateWorldTransform();
	}
}

void Actor::AddChild(Actor* child)
{
	mChildren.push_back(child);
}

void Actor::RemoveChild(Actor* child)
{
	auto it = std::find(mChildren.begin(), mChildren.end(), child);
	if(it != mChildren.end())
	{
		mChildren.erase(it);
	}
}
