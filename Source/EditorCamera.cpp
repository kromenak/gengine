//
// EditorCamera.cpp
//
// Clark Kromenaker
//
#include "EditorCamera.h"
#include "CameraComponent.h"

const float kCameraSpeed = 100.0f;
const float kRunCameraMultiplier = 2.0f;
const float kCameraRotationSpeed = 2.5f;

EditorCamera::EditorCamera()
{
    AddComponent<CameraComponent>();
}

void EditorCamera::Update(float deltaTime)
{
    // Determine camera speed.
    float camSpeed = kCameraSpeed;
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LSHIFT))
    {
        camSpeed = kCameraSpeed * kRunCameraMultiplier;
    }
    
    // Forward and backward movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_W))
    {
        Translate(GetForward() * (camSpeed * deltaTime));
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_S))
    {
        Translate(GetForward() * (-camSpeed * deltaTime));
    }
    
    // Up and down movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_E))
    {
        Translate(Vector3(0.0f, camSpeed * deltaTime, 0.0f));
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_Q))
    {
        Translate(Vector3(0.0f, -camSpeed * deltaTime, 0.0f));
    }
    
    // Strafe left and right movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_A))
    {
        Rotate(Vector3::UnitY, -kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_D))
    {
        Rotate(Vector3::UnitY, kCameraRotationSpeed * deltaTime);
    }
}
