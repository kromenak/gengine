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

void CameraComponent::Update(float deltaTime)
{
    if(Services::GetInput()->IsPressed(SDL_SCANCODE_W))
    {
        mOwner->Translate(Vector3(0.0f, 0.0f, -1.0f * deltaTime));
    }
    else if(Services::GetInput()->IsPressed(SDL_SCANCODE_S))
    {
        mOwner->Translate(Vector3(0.0f, 0.0f, 1.0f * deltaTime));
    }
}

Matrix4 CameraComponent::GetLookAtMatrix()
{
    Vector3 eye = mOwner->GetPosition();
    Vector3 lookAt = mOwner->GetPosition() - Vector3::UnitZ * 5.0f;
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
