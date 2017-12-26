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

void CameraComponent::Render()
{
    
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
    Matrix4 projMat = Matrix4::MakePerspective(1.74533f, 1.333f, 0.0f, 100.0f);
    return projMat;
}
