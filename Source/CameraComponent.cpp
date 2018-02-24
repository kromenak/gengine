//
// CameraComponent.cpp
//
// Clark Kromenaker
//
#include "CameraComponent.h"
#include "Services.h"
#include "SDLRenderer.h"

const float kCameraSpeed = 50.0f;
const float kCameraRotationSpeed = 1.0f;

CameraComponent::CameraComponent(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->SetCamera(this);
}

void CameraComponent::Update(float deltaTime)
{
    if(Services::GetInput()->IsPressed(SDL_SCANCODE_W))
    {
        mOwner->Translate(mOwner->GetForward() * (kCameraSpeed * deltaTime));
    }
    else if(Services::GetInput()->IsPressed(SDL_SCANCODE_S))
    {
        mOwner->Translate(mOwner->GetForward() * (-kCameraSpeed * deltaTime));
    }
    
    if(Services::GetInput()->IsPressed(SDL_SCANCODE_Q))
    {
        mOwner->Translate(Vector3(0.0f, kCameraSpeed * deltaTime, 0.0f));
    }
    else if(Services::GetInput()->IsPressed(SDL_SCANCODE_E))
    {
        mOwner->Translate(Vector3(0.0f, -kCameraSpeed * deltaTime, 0.0f));
    }
    
    if(Services::GetInput()->IsPressed(SDL_SCANCODE_A))
    {
        mOwner->Rotate(Vector3::UnitY, kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsPressed(SDL_SCANCODE_D))
    {
        mOwner->Rotate(Vector3::UnitY, -kCameraRotationSpeed * deltaTime);
    }
}

Matrix4 CameraComponent::GetLookAtMatrix()
{
    Vector3 eye = mOwner->GetPosition();
    Vector3 lookAt = mOwner->GetPosition() + mOwner->GetForward() * 5.0f;
    //Vector3 lookAt = mOwner->GetPosition() - Vector3::UnitZ * 5.0f;
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
