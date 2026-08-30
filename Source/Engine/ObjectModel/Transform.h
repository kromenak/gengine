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
    TYPEINFO_SUB(Transform, Component);
public:
    enum class Space
    {
        Local,	// This transform's local space
        World	// World space
    };

    Transform(Actor* owner);
    virtual ~Transform();

    // Local position/rotation/scale/axes accessors.
    const Vector3& GetPosition() const { return mLocalPosition; }
    void SetPosition(const Vector3& position);

    const Quaternion& GetRotation() const { return mLocalRotation; }
    void SetRotation(const Quaternion& rotation);

    const Vector3& GetScale() const { return mLocalScale; }
    void SetScale(const Vector3& scale);

    // NOTE: this is where we decide which axes correlate to forward/right/up in world space.
    Vector3 GetForward() const { return mLocalRotation.Rotate(Vector3::UnitZ); }
    Vector3 GetRight() const { return mLocalRotation.Rotate(Vector3::UnitX); }
    Vector3 GetUp() const { return mLocalRotation.Rotate(Vector3::UnitY); }

    Vector3 GetWorldForward() const { return GetWorldRotation().Rotate(Vector3::UnitZ); }
    Vector3 GetWorldRight() const { return GetWorldRotation().Rotate(Vector3::UnitX); }
    Vector3 GetWorldUp() const { return GetWorldRotation().Rotate(Vector3::UnitY); }

    // World position/rotation/scale/axes accessors.
    Vector3 GetWorldPosition() const;
    void SetWorldPosition(const Vector3& position);

    Quaternion GetWorldRotation() const;
    void SetWorldRotation(const Quaternion& rotation);

    Vector3 GetWorldScale() const;

    // Parenting.
    void SetParent(Transform* parent);
    Transform* GetParent() const { return mParent; }
    const std::vector<Transform*>& GetChildren() const { return mChildren; }

    // Transform matrices.
    const Matrix4& GetLocalToWorldMatrix() const;
    const Matrix4& GetWorldToLocalMatrix() const;

    // Transforms points/directions from local space to world space.
    Vector3 LocalToWorldPoint(const Vector3& localPoint) const;
    Vector3 LocalToWorldDirection(const Vector3& localDirection) const;
    Quaternion LocalToWorldRotation(const Quaternion& localRotation) const;

    // Transforms points/directions from world space to local space.
    Vector3 WorldToLocalPoint(const Vector3& worldPoint) const;
    Vector3 WorldToLocalDirection(const Vector3& worldDirection) const;
    //WorldToLocalRotation(...);

    // Changes position by some offset
    void Translate(const Vector3& offset);

    // Rotates about local space origin about an axis/angle defined in either local or world space.
    void Rotate(const Vector3& axis, float angle, Space space = Space::Local);
    void Rotate(const Quaternion& rotation, Space space = Space::Local);

    // Rotates about an axis at a world point, by an angle.
    void RotateAround(const Vector3& worldPoint, const Vector3& axis, float angle);
    void RotateAround(const Vector3& worldPoint, const Quaternion& rotation);

    virtual void SetDirty();

protected:
    // Local position, rotation, and scale. Relative to parent's position/rotation/scale.
    // Only equal to world position/rotation/scale if I have no parent.
    Vector3 mLocalPosition;
    Quaternion mLocalRotation;
    Vector3 mLocalScale;

    // If we are a child of any other transform, parent is set.
    // If we have any children, they are in the children vector.
    Transform* mParent = nullptr;
    std::vector<Transform*> mChildren;

private:
    // Our local-to-world matrix, and our world-to-local matrix.
    // World-to-local is just the inverse of local-to-world!
    mutable Matrix4 mLocalToWorldMatrix = Matrix4::Identity;
    mutable Matrix4 mWorldToLocalMatrix = Matrix4::Identity;

    // We only recalculate our matrices when we have to. This keeps track of that.
    mutable bool mLocalToWorldDirty = true;
    mutable bool mWorldToLocalDirty = true;
};
