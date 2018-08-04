//
// CameraComponent.h
//
// Clark Kromenaker
//
#pragma once
#include "Component.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector2.h"

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
    float fovAngleRad = 1.3264f; //1.0472f; //1.74533f;
    
    // Near and far clipping planes, for any projection type.
    float nearClippingPlane = 1.0f;
    float farClippingPlane = 10000.0f;
    
    Matrix4 MakeLookAt(const Vector3& eye, const Vector3& lookAt, const Vector3& up);
};
