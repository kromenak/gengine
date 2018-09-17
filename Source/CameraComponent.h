//
// CameraComponent.h
//
// Clark Kromenaker
//
#pragma once
#include "Component.h"

#include "Matrix4.h"
#include "Vector2.h"
#include "Vector3.h"

class CameraComponent : public Component
{
    TYPE_DECL_CHILD();
public:
    CameraComponent(Actor* owner);
    
    Matrix4 GetLookAtMatrix();
    Matrix4 GetLookAtMatrixNoTranslate();
    Matrix4 GetProjectionMatrix();
    
    Vector3 ScreenToWorldPoint(const Vector2& screenPoint, float distance);
    
private:
    // Field of view angle, in radians, for perspective projection.
    float mFovAngleRad = 1.3264f;
    
    // Near and far clipping planes, for any projection type.
    float mNearClipPlane = 1.0f;
    float mFarClipPlane = 10000.0f;
};
