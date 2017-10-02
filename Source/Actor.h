//
// Actor.h
//
// Clark Kromenaker
//
// Description goes here!
//

#pragma once
#include "Vector3.h"
#include "Matrix4.h"

class Actor
{
public:
    Actor();
    
    Vector3 GetPosition() const { return mPosition; }
    void SetPosition(Vector3 position);
    
    Vector3 GetRotation() const { return mRotation; }
    void SetRotation(Vector3 rotation);
    
    Vector3 GetScale() const { return mScale; }
    void SetScale(Vector3 scale);
    
    Matrix4 GetWorldTransformMatrix() const { return mWorldTransform; }
    
private:
    // NOTE: Based on GK3 MOD files, it appears that GK3 uses a coordinate system where:
    // Forward = -Y
    // Up = +Z
    // Right (when facing +Y) = +X
    Vector3 mPosition;
    Vector3 mRotation;
    Vector3 mScale;
    
    Matrix4 mWorldTransform;
    
    void UpdateWorldTransform();
};
