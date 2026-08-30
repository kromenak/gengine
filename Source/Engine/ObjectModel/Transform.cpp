#include "Transform.h"

#include "Log.h"

TYPEINFO_INIT(Transform, Component, 2)
{
    TYPEINFO_VAR(Transform, VariableType::Vector3, mLocalPosition);
    TYPEINFO_VAR(Transform, VariableType::Quaternion, mLocalRotation);
    TYPEINFO_VAR(Transform, VariableType::Vector3, mLocalScale);
}

Transform::Transform(Actor* owner) : Component(owner),
    mLocalPosition(0.0f, 0.0f, 0.0f),
    mLocalRotation(0.0f, 0.0f, 0.0f, 1.0f),
    mLocalScale(1.0f, 1.0f, 1.0f)
{

}

Transform::~Transform()
{
    // Ensure that deleted actor doesn't stay a child of some actor.
    SetParent(nullptr);

    // If this actor is gone...what about all its children? For now, just unparent the child entirely.
    // Calling SetParent(nullptr) will remove from this list until it is empty.
    while(!mChildren.empty())
    {
        mChildren.back()->SetParent(nullptr);
    }
}

void Transform::SetPosition(const Vector3& position)
{
    mLocalPosition = position;
    SetDirty();
}

void Transform::SetRotation(const Quaternion& rotation)
{
    mLocalRotation = rotation;
    SetDirty();
}

void Transform::SetScale(const Vector3& scale)
{
    mLocalScale = scale;
    SetDirty();
}

Vector3 Transform::GetWorldPosition() const
{
    if(mParent != nullptr)
    {
        return mParent->GetLocalToWorldMatrix().TransformPoint(mLocalPosition);
    }
    return mLocalPosition;
}

void Transform::SetWorldPosition(const Vector3& position)
{
    if(mParent != nullptr)
    {
        mLocalPosition = mParent->GetWorldToLocalMatrix().TransformPoint(position);
    }
    else
    {
        mLocalPosition = position;
    }
    SetDirty();
}

Quaternion Transform::GetWorldRotation() const
{
    if(mParent != nullptr)
    {
        // If we have a parent, we must take their rotation and then apply ours relative to theirs.
        // Going up the chain, this ultimately gets us a combined rotation in world space.
        return mParent->GetWorldRotation() * mLocalRotation;
    }

    // With no parent, assumed to be in world space already.
    return mLocalRotation;
}

void Transform::SetWorldRotation(const Quaternion& rotation)
{
    if(mParent != nullptr)
    {
        // If we have a parent, we must convert the world space rotation to local space.
        // This can be done by multiplying with the inverse of the parent's rotation.
        mLocalRotation = Quaternion::Inverse(mParent->GetWorldRotation()) * rotation;
    }
    else
    {
        // With no parent our local rotation is already in world space.
        mLocalRotation = rotation;
    }
    SetDirty();
}

Vector3 Transform::GetWorldScale() const
{
    if(mParent != nullptr)
    {
        // This works, but gives incorrect results with non-uniform scale.
        // return mParent->GetWorldScale() * mLocalScale;

        // To deal with non-uniform scale effectively, we should extract scales directly from the columns of the world transform matrix.
        const Matrix4& localToWorldMatrix = GetLocalToWorldMatrix();
        return Vector3(localToWorldMatrix[0].GetLength(), localToWorldMatrix[1].GetLength(), localToWorldMatrix[2].GetLength());
    }
    return mLocalScale;
}

void Transform::SetParent(Transform* parent)
{
    // I can't be the child of a parent if the parent is my child...
    // In other words, disallow using a parent that is my child.
    Transform* parentAncestor = parent;
    while(parentAncestor != nullptr)
    {
        if(parentAncestor == this)
        {
            Logf("Cyclical transform hierarchy detected and avoided!");
            return;
        }
        parentAncestor = parentAncestor->mParent;
    }

    // Remove from existing parent.
    if(mParent != nullptr)
    {
        auto it = std::find(mParent->mChildren.begin(), mParent->mChildren.end(), this);
        if(it != mParent->mChildren.end())
        {
            mParent->mChildren.erase(it);
        }
        mParent = nullptr;
    }

    //TODO: The current logic keeps this transform's local position even though the parent changed.
    //TODO: So it's likely the transform will "snap" to a new world position on next render.
    //TODO: We could have an option to have an object keep its world position when changing a parent?

    // Attach to new parent. It could be null for "no parent".
    mParent = parent;
    if(mParent != nullptr)
    {
        mParent->mChildren.push_back(this);
    }

    // Changing parent requires recalculating matrices.
    SetDirty();
}

const Matrix4& Transform::GetLocalToWorldMatrix() const
{
    if(mLocalToWorldDirty)
    {
        // Get translate/rotate/scale matrices.
        Matrix4 translateMatrix = Matrix4::MakeTranslate(mLocalPosition);
        Matrix4 rotateMatrix = Matrix4::MakeRotate(mLocalRotation);
        Matrix4 scaleMatrix = Matrix4::MakeScale(mLocalScale);

        // Combine in order (Scale, Rotate, Translate) to generate world transform matrix.
        mLocalToWorldMatrix = translateMatrix * rotateMatrix * scaleMatrix;

        // If I'm a child, multiply parent transform into the mix.
        if(mParent != nullptr)
        {
            mLocalToWorldMatrix = mParent->GetLocalToWorldMatrix() * mLocalToWorldMatrix;
        }

        // The local to world matrix is no longer dirty.
        mLocalToWorldDirty = false;

        // The world to local matrix is calculated from the local to world matrix.
        // So, any update will dirty the world to local matrix!
        mWorldToLocalDirty = true;
    }
    return mLocalToWorldMatrix;
}

const Matrix4& Transform::GetWorldToLocalMatrix() const
{
    if(mWorldToLocalDirty)
    {
        mWorldToLocalMatrix = Matrix4::InverseTransform(GetLocalToWorldMatrix());
        mWorldToLocalDirty = false;
    }
    return mWorldToLocalMatrix;
}

Vector3 Transform::LocalToWorldPoint(const Vector3& localPoint) const
{
    return GetLocalToWorldMatrix().TransformPoint(localPoint);
}

Vector3 Transform::LocalToWorldDirection(const Vector3& localDirection) const
{
    // If there is scale applied to this transform or any parent, the scale will be applied to the transformed direction.
    // As a result, we should normalize before returning - this is meant to calculate a unit direction vector.
    return GetLocalToWorldMatrix().TransformVector(localDirection).Normalize();
}

Quaternion Transform::LocalToWorldRotation(const Quaternion& localRotation) const
{
    // This is basically the same code/idea that's in GetWorldPosition.
    // Take world rotation and then apply ours relative to that.
    return GetWorldRotation() * localRotation;
}

Vector3 Transform::WorldToLocalPoint(const Vector3& worldPoint) const
{
    return GetWorldToLocalMatrix().TransformPoint(worldPoint);
}

Vector3 Transform::WorldToLocalDirection(const Vector3& worldDirection) const
{
    // As with LocalToWorldDirection, make sure result is unit length despite any scaling on intermediate transforms.
    return GetWorldToLocalMatrix().TransformVector(worldDirection).Normalize();
}

void Transform::Translate(const Vector3& offset)
{
    SetPosition(mLocalPosition + offset);
}

void Transform::Rotate(const Vector3& axis, float angle, Space space)
{
    Rotate(Quaternion(axis, angle), space);
}

void Transform::Rotate(const Quaternion& rotation, Space space)
{
    if(space == Space::Local)
    {
        // This takes our current local rotation and applies the passed in rotation on top of it.
        SetRotation(mLocalRotation * rotation);
    }
    else
    {
        // Apply the passed in rotation first (in world space) and then apply our existing rotation on top of it.
        SetWorldRotation(rotation * GetWorldRotation());
    }
}

void Transform::RotateAround(const Vector3& worldPoint, const Vector3& axis, float angle)
{
    RotateAround(worldPoint, Quaternion(axis, angle));
}

void Transform::RotateAround(const Vector3& worldPoint, const Quaternion& rotation)
{
    // HOW THIS WORKS: rotating a transform normally does not change the transform's position - we just rotate about the transform's origin.
    // To rotate about some other point, the transform's position must change during the rotation to keep positions the same relative to that point.
    // 1) Calc offset from rotation point.
    // 2) Rotate that offset by desired rotation.
    // 3) Move transform to point + offset.
    // 4) Rotate as you would normally.

    // Calculate world-space offset from point rotating around to current position.
    Vector3 worldPos = GetWorldPosition();
    Vector3 pointToPos = worldPos - worldPoint;

    // Rotate offset from rotation point to old world pos to get a new offset.
    // Adding that new offset to the rotation point gives us the object's new world space position.
    SetWorldPosition(worldPoint + rotation.Rotate(pointToPos));

    // Actually rotate the transform.
    Rotate(rotation, Transform::Space::World);
}

void Transform::SetDirty()
{
    mLocalToWorldDirty = true;
    mWorldToLocalDirty = true;

    for(auto& child : mChildren)
    {
        child->SetDirty();
    }
}

