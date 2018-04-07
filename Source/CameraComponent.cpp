//
// CameraComponent.cpp
//
// Clark Kromenaker
//
#include "CameraComponent.h"
#include "Services.h"
#include "SDLRenderer.h"

CameraComponent::CameraComponent(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

Matrix4 CameraComponent::GetLookAtMatrix()
{
    Vector3 eye = mOwner->GetPosition();
    Vector3 lookAt = mOwner->GetPosition() + mOwner->GetForward() * 5.0f;
    Vector3 up = Vector3::UnitY;
    Matrix4 lookAtMat = Matrix4::MakeLookAt(eye, lookAt, up);
    return lookAtMat;
}

Matrix4 CameraComponent::GetProjectionMatrix()
{
    Matrix4 projMat = Matrix4::MakePerspective(fovAngleRad, 1.333f,
                                               nearClippingPlane,
                                               farClippingPlane);
    return projMat;
}
