#include "Transform.h"

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

    // If this actor is gone...what about all its children?
    // For now, let's just unparent the child entirely! (Maybe should set to my parent instead?)
    for(auto& child : mChildren)
    {
        child->mParent = nullptr;
        child->SetDirty();
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
        return mParent->GetLocalToWorldMatrix().TransformPoint(GetPosition());
    }
    return GetPosition();
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
        return mParent->GetWorldRotation() * GetRotation();
    }
    return GetRotation();
}

void Transform::SetWorldRotation(const Quaternion& rotation)
{
    if(mParent != nullptr)
    {
        mLocalRotation = Quaternion::Inverse(mParent->GetWorldRotation()) * rotation;
    }
    else
    {
        mLocalRotation = rotation;
    }
}

Vector3 Transform::GetWorldScale() const
{
    if(mParent != nullptr)
    {
        return mParent->GetWorldScale() * mLocalScale;
    }
    return mLocalScale;
}

void Transform::SetParent(Transform* parent)
{
    // Remove from existing parent.
    if(mParent != nullptr)
    {
        mParent->RemoveChild(this);
        mParent = nullptr;
    }

    //TODO: Ensure not setting as parent one of my children?
    //TODO: For now, let's count on not doing that...

    // Attach to new parent. It could be null for "no parent".
    mParent = parent;
    if(mParent != nullptr)
    {
        mParent->AddChild(this);
    }

    // Changing parent requires recalculating matrices.
    SetDirty();
}

const Matrix4& Transform::GetLocalToWorldMatrix()
{
    if(mLocalToWorldDirty)
    {
        // Make sure local position is up-to-date.
        // This is primarily for RectTransform pivot/size changing local position.
        CalcLocalPosition();

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

const Matrix4& Transform::GetWorldToLocalMatrix()
{
    if(mWorldToLocalDirty)
    {
        mWorldToLocalMatrix = Matrix4::InverseTransform(GetLocalToWorldMatrix());
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
    Vector4 result = Vector4(localDirection.x, localDirection.y, localDirection.z, 0.0f) * GetWorldToLocalMatrix();
    return Vector3(result.x, result.y, result.z);
}

Quaternion Transform::LocalToWorldRotation(const Quaternion& localRotation)
{
    return GetWorldRotation() * localRotation;
}

Vector3 Transform::WorldToLocalPoint(const Vector3& worldPoint)
{
    return GetWorldToLocalMatrix().TransformPoint(worldPoint);
}

Vector3 Transform::WorldToLocalDirection(const Vector3& worldDirection)
{
    //TODO: Unit test this?
    Vector4 result = Vector4(worldDirection.x, worldDirection.y, worldDirection.z, 0.0f) * GetLocalToWorldMatrix();
    return Vector3(result.x, result.y, result.z);
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
        SetRotation(GetRotation() * rotation);
    }
    else
    {
        // What's going on here!? From left to right:
        // 1) WR * IWR cancels each other out - gets us to zero rotation
        // 2) Apply desired rotation
        // 3) Reapply previous WR on top of rotation
        Quaternion worldRotation = GetWorldRotation();
        Quaternion invertedWorldRotation = Quaternion::Inverse(worldRotation);
        SetWorldRotation(worldRotation * invertedWorldRotation * rotation * worldRotation);
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
    Rotate(rotation);
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

