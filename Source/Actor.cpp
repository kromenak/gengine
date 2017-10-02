//
// Actor.cpp
//
// Clark Kromenaker
//

#include "Actor.h"

Actor::Actor() : mPosition(Vector3(0, 0, 0)),
    mRotation(Vector3(0, 0, 0)),
    mScale(Vector3(1, 1, 1))
{
    UpdateWorldTransform();
}

void Actor::SetPosition(Vector3 position)
{
    mPosition = position;
    UpdateWorldTransform();
}

void Actor::SetRotation(Vector3 rotation)
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
    Matrix4 translateMatrix = Matrix4::MakeTranslateMatrix(mPosition);
    Matrix4 rotateXMatrix = Matrix4::MakeRotateXMatrix(mRotation.GetX());
    Matrix4 rotateYMatrix = Matrix4::MakeRotateYMatrix(mRotation.GetY());
    Matrix4 rotateZMatrix = Matrix4::MakeRotateZMatrix(mRotation.GetZ());
    Matrix4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
    Matrix4 scaleMatrix = Matrix4::MakeScaleMatrix(mScale);
    
    mWorldTransform = scaleMatrix * rotateMatrix * translateMatrix;
    
    cout << mWorldTransform << endl;
}
