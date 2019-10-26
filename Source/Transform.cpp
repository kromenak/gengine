//
// Transform.cpp
//
// Clark Kromenaker
//
#include "Transform.h"

TYPE_DEF_CHILD(Component, Transform);

Transform::Transform(Actor* owner) : Component(owner),
	mLocalPosition(0, 0, 0), mLocalRotation(0, 0, 0, 1), mLocalScale(1, 1, 1)
{
	
}

void Transform::Translate(Vector3 offset)
{
	SetPosition(mLocalPosition + offset);
}

void Transform::Rotate(Vector3 axis, float angle)
{
	SetRotation(Quaternion(axis, angle) * GetRotation());
}

void Transform::SetPosition(Vector3 position)
{
	mLocalPosition = position;
	SetDirty();
}

void Transform::SetRotation(Quaternion rotation)
{
	mLocalRotation = rotation;
	SetDirty();
}

void Transform::SetScale(Vector3 scale)
{
	mLocalScale = scale;
	SetDirty();
}

Vector3 Transform::GetWorldPosition() const
{
	if(mParent != nullptr)
	{
		return mParent->GetLocalToWorldMatrix().TransformPoint(GetPosition());
	}
	return GetPosition();
}

Quaternion Transform::GetWorldRotation() const
{
	if(mParent != nullptr)
	{
		return mParent->GetWorldRotation() * GetRotation();
	}
	return GetRotation();
}

void Transform::SetParent(Transform* parent)
{
	// Remove from existing parent.
	if(mParent != nullptr)
	{
		mParent->RemoveChild(this);
		mParent = nullptr;
	}
	
	//TODO: Ensure not setting as parent one of my children.
	
	// Attach to new parent. It could be null for "no parent".
	mParent = parent;
	if(mParent != nullptr)
	{
		mParent->AddChild(this);
	}
	
	// Changing parent requires recalculating matrices.
	SetDirty();
}

Matrix4 Transform::GetLocalToWorldMatrix()
{
	if(mLocalToWorldDirty)
	{
		// Get translate/rotate/scale matrices.
		Matrix4 translateMatrix = Matrix4::MakeTranslate(GetLocalPosition());
		Matrix4 rotateMatrix = Matrix4::MakeRotate(mLocalRotation);
		Matrix4 scaleMatrix = Matrix4::MakeScale(mLocalScale);
		
		// Combine in order (Scale, Rotate, Translate) to generate world transform matrix.
		mLocalToWorldMatrix = translateMatrix * rotateMatrix * scaleMatrix;
		
		// If I'm a child, multiply parent transform into the mix.
		if(mParent != nullptr)
		{
			mLocalToWorldMatrix = mParent->GetLocalToWorldMatrix() * mLocalToWorldMatrix;
		}
		
		mLocalToWorldDirty = false;
	}
	return mLocalToWorldMatrix;
}

Matrix4 Transform::GetWorldToLocalMatrix()
{
	if(mWorldToLocalDirty)
	{
		mWorldToLocalMatrix = GetLocalToWorldMatrix().Inverse();
		mWorldToLocalDirty = false;
	}
	return mWorldToLocalMatrix;
}

Vector3 Transform::LocalToWorldPoint(const Vector3& localPoint)
{
	return GetLocalToWorldMatrix().TransformPoint(localPoint);
}

Vector3 Transform::LocalToWorldDirection(const Vector3& localDirection)
{
	//TODO: Unit test this?
	Vector4 result = Vector4(localDirection.GetX(), localDirection.GetY(), localDirection.GetZ(), 0.0f) * GetWorldToLocalMatrix();
	return Vector3(result.GetX(), result.GetY(), result.GetZ());
}

Vector3 Transform::WorldToLocalPoint(const Vector3& worldPoint)
{
	return GetWorldToLocalMatrix().TransformPoint(worldPoint);
}

Vector3 Transform::WorldToLocalDirection(const Vector3& worldDirection)
{
	//TODO: Unit test this?
	Vector4 result = Vector4(worldDirection.GetX(), worldDirection.GetY(), worldDirection.GetZ(), 0.0f) * GetLocalToWorldMatrix();
	return Vector3(result.GetX(), result.GetY(), result.GetZ());
}

Vector3 Transform::GetLocalPosition()
{
	return mLocalPosition;
}

void Transform::SetDirty()
{
	if(!mLocalToWorldDirty)
	{
		mLocalToWorldDirty = true;
		mWorldToLocalDirty = true;
		
		for(auto& child : mChildren)
		{
			child->SetDirty();
		}
	}
}

void Transform::AddChild(Transform* child)
{
	mChildren.push_back(child);
}

void Transform::RemoveChild(Transform* child)
{
	auto it = std::find(mChildren.begin(), mChildren.end(), child);
	if(it != mChildren.end())
	{
		mChildren.erase(it);
	}
}
