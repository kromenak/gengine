//
// Transform.h
//
// Clark Kromenaker
//
// Manages an actor's position, rotation, and scale
// and hierarchy of actors in the scene (parents, children, etc).
//
#pragma once
#include "Component.h"

#include <vector>

#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"

class Transform : public Component
{
	TYPE_DECL_CHILD();
public:
	Transform(Actor* owner);
	
	void Translate(Vector3 offset);
	void Rotate(Vector3 axis, float angle);
	
	Vector3 GetPosition() const { return mLocalPosition; }
	void SetPosition(Vector3 position);
	
	Quaternion GetRotation() const { return mLocalRotation; }
	void SetRotation(Quaternion rotation);
	
	Vector3 GetScale() const { return mLocalScale; }
	void SetScale(Vector3 scale);
	
	Vector3 GetWorldPosition() const;
	Quaternion GetWorldRotation() const;
	
	Vector3 GetForward() const { return mLocalRotation.Rotate(Vector3::UnitZ); }
	Vector3 GetRight() const { return mLocalRotation.Rotate(Vector3::UnitX); }
	Vector3 GetUp() const { return mLocalRotation.Rotate(Vector3::UnitY); }
	
	void SetParent(Transform* parent);
	Transform* GetParent() const { return mParent; }
	
	Matrix4 GetLocalToWorldMatrix();
	Matrix4 GetWorldToLocalMatrix();
	
	// Transforms points/directions from local space to world space.
	Vector3 LocalToWorldPoint(const Vector3& localPoint);
	Vector3 LocalToWorldDirection(const Vector3& localDirection);
	
	// Transforms points/directions from world space to local space.
	Vector3 WorldToLocalPoint(const Vector3& worldPoint);
	Vector3 WorldToLocalDirection(const Vector3& worldDirection);
	
protected:
	// Local position, rotation, and scale.
	Vector3 mLocalPosition;
	Quaternion mLocalRotation;
	Vector3 mLocalScale;
	
	// Our local-to-world matrix, and our world-to-local matrix.
	// World-to-local is just the inverse of local-to-world!
	Matrix4 mLocalToWorldMatrix;
	Matrix4 mWorldToLocalMatrix;
	
	// We only recalculate our matrices when we have to. This keeps track of that.
	bool mLocalToWorldDirty = true;
	bool mWorldToLocalDirty = true;
	
	// A parent, if any.
	Transform* mParent = nullptr;
	
	// Children, if any.
	std::vector<Transform*> mChildren;
	
	void SetDirty();
	
	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
};
