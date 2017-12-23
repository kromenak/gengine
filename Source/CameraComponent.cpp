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

void CameraComponent::Render()
{
    
}
