//
// GameCamera.cpp
//
// Clark Kromenaker
//
#include "GameCamera.h"

#include "AudioListener.h"
#include "Camera.h"
#include "GEngine.h"
#include "Scene.h"

const float kCameraSpeed = 100.0f;
const float kRunCameraMultiplier = 2.0f;
const float kCameraRotationSpeed = 2.5f;

GameCamera::GameCamera()
{
    mCamera = AddComponent<Camera>();
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
        GetTransform()->Translate(GetForward() * (camSpeed * deltaTime));
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_S))
    {
        GetTransform()->Translate(GetForward() * (-camSpeed * deltaTime));
    }
    
    // Up and down movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_E))
    {
        GetTransform()->Translate(Vector3(0.0f, camSpeed * deltaTime, 0.0f));
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_Q))
    {
        GetTransform()->Translate(Vector3(0.0f, -camSpeed * deltaTime, 0.0f));
    }
    
    // Rotate left and right movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_A))
    {
        GetTransform()->Rotate(Vector3::UnitY, -kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_D))
    {
        GetTransform()->Rotate(Vector3::UnitY, kCameraRotationSpeed * deltaTime);
    }
    
    // Rotate up and down movement.
    if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_C))
    {
        GetTransform()->Rotate(GetRight(), -kCameraRotationSpeed * deltaTime);
    }
    else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_Z))
    {
        GetTransform()->Rotate(GetRight(), kCameraRotationSpeed * deltaTime);
    }
	
	if(mCamera != nullptr)
	{
		// Calculate mouse click ray.
		Vector2 mousePos = Services::GetInput()->GetMousePosition();
		Vector3 worldPos = mCamera->ScreenToWorldPoint(mousePos, 0.0f);
		Vector3 worldPos2 = mCamera->ScreenToWorldPoint(mousePos, 1.0f);
		Vector3 dir = (worldPos2 - worldPos).Normalize();
		Ray ray(worldPos, dir);
		
		// If we can interact with whatever we are pointing at, highlight the cursor.
		bool canInteract = GEngine::inst->GetScene()->CheckInteract(ray);
		if(canInteract)
		{
			GEngine::inst->UseHighlightCursor();
		}
		else
		{
			GEngine::inst->UseDefaultCursor();
		}
		
		// If left mouse button is released, try to interact with whatever it is over.
		// Need to do this, even if canInteract==false, because floor can be clicked to move around.
		if(Services::GetInput()->IsMouseButtonUp(InputManager::MouseButton::Left))
		{
			GEngine::inst->GetScene()->Interact(ray);
		}
	}
	
	// For debugging, output camera position.
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_P))
	{
		std::cout << GetPosition() << std::endl;
	}
}
