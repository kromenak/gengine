//
// GameCamera.cpp
//
// Clark Kromenaker
//
#include "GameCamera.h"

#include "AudioListener.h"
#include "CameraComponent.h"
#include "GEngine.h"
#include "Scene.h"

const float kCameraSpeed = 100.0f;
const float kRunCameraMultiplier = 2.0f;
const float kCameraRotationSpeed = 2.5f;

GameCamera::GameCamera()
{
    mCamera = AddComponent<CameraComponent>();
    AddComponent<AudioListener>();
}

void GameCamera::UpdateInternal(float deltaTime)
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
    
    // Rotate left and right movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_A))
    {
        Rotate(Vector3::UnitY, -kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_D))
    {
        Rotate(Vector3::UnitY, kCameraRotationSpeed * deltaTime);
    }
    
    // Rotate up and down movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_C))
    {
        Rotate(GetRight(), -kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_Z))
    {
        Rotate(GetRight(), kCameraRotationSpeed * deltaTime);
    }
    
    if(Services::GetInput()->IsMouseButtonDown(InputManager::MouseButton::Left))
    {
        if(mCamera != nullptr)
        {
            // Calculate mouse click ray.
            Vector2 mousePos = Services::GetInput()->GetMousePosition();
            Vector3 worldPos = mCamera->ScreenToWorldPoint(mousePos, 0.0f);
            Vector3 worldPos2 = mCamera->ScreenToWorldPoint(mousePos, 1.0f);
            Vector3 dir = (worldPos2 - worldPos).Normalize();
            Ray ray(worldPos, dir);
            
            GEngine::inst->GetScene()->Interact(ray);
        }
    }
}
