//
// CameraComponent.h
//
// Clark Kromenaker
//
#pragma once
#include "Component.h"
#include "Matrix4.h"

class CameraComponent : public Component
{
public:
    CameraComponent(Actor* owner);
    
    void Update(float deltaTime) override;
    
    Matrix4 GetLookAtMatrix();
    Matrix4 GetProjectionMatrix();
    
private:
    // Field of view angle, in radians, for perspective projection.
    float fovAngleRad = 1.74533f;
    
    // Near and far clipping planes, for any projection type.
    float nearClippingPlane = 0.01f;
    float farClippingPlane = 1000.0f;
};
