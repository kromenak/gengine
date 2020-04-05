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
#include "SphereCollider.h"
#include "UICanvas.h"

GameCamera::GameCamera()
{
    mCamera = AddComponent<Camera>();
    AddComponent<AudioListener>();
	AddComponent<SphereCollider>();
}

void GameCamera::SetAngle(const Vector2& angle)
{
	SetAngle(angle.GetX(), angle.GetY());
}

void GameCamera::SetAngle(float yaw, float pitch)
{
	SetRotation(Quaternion(Vector3::UnitY, yaw) * Quaternion(Vector3::UnitX, pitch));
}

void GameCamera::OnUpdate(float deltaTime)
{
	// We don't move/turn unless some input causes it.
	float forwardSpeed = 0.0f;
	float strafeSpeed = 0.0f;
	float turnSpeed = 0.0f;
	float pitchSpeed = 0.0f;
	float verticalSpeed = 0.0f;
	
	// Pan/Pan modifiers are activated with CTRL/SHIFT keys.
	// These work EVEN IF text input is active.
	bool panModifierActive = Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LCTRL) ||
							 Services::GetInput()->IsKeyPressed(SDL_SCANCODE_RCTRL);;
	bool pitchModifierActive = Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LSHIFT) ||
							   Services::GetInput()->IsKeyPressed(SDL_SCANCODE_RSHIFT);
	
	// W/S/A/D and Up/Down/Left/Right only work if text input isn't stealing input.
	if(!Services::GetInput()->IsTextInput())
	{
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_W) ||
		   Services::GetInput()->IsKeyPressed(SDL_SCANCODE_UP))
		{
			if(pitchModifierActive)
			{
				pitchSpeed += kRotationSpeed;
			}
			else if(panModifierActive)
			{
				verticalSpeed += kSpeed;
			}
			else
			{
				forwardSpeed += kSpeed;
			}
		}
		else if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_S) ||
				Services::GetInput()->IsKeyPressed(SDL_SCANCODE_DOWN))
		{
			if(pitchModifierActive)
			{
				pitchSpeed -= kRotationSpeed;
			}
			else if(panModifierActive)
			{
				verticalSpeed -= kSpeed;
			}
			else
			{
				forwardSpeed -= kSpeed;
			}
		}
		
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_D) ||
		   Services::GetInput()->IsKeyPressed(SDL_SCANCODE_RIGHT))
		{
			if(pitchModifierActive)
			{
				turnSpeed += kRotationSpeed;
			}
			else if(panModifierActive)
			{
				strafeSpeed += kSpeed;
			}
			else
			{
				turnSpeed += kRotationSpeed;
			}
		}
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_A) ||
		   Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LEFT))
		{
			if(pitchModifierActive)
			{
				turnSpeed -= kRotationSpeed;
			}
			else if(panModifierActive)
			{
				strafeSpeed -= kSpeed;
			}
			else
			{
				turnSpeed -= kRotationSpeed;
			}
		}
	}
	
	// If left mouse button is held down, mouse movement contributes to camera movement.
	bool leftMousePressed = Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Left);
	if(leftMousePressed)
	{
		// Pan modifier also activates if right mouse button is pressed.
		panModifierActive |= Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Right);
		
		// Mouse delta is in pixels.
		// We normalize this by estimating "max pixel movement" per frame.
		Vector2 mouseDelta = Services::GetInput()->GetMouseDelta();
		mouseDelta /= kMouseRangePixels;
		
		// Lock the mouse!
		if(mouseDelta.GetLengthSq() > 0.0f)
		{
			Services::GetInput()->LockMouse();
		}
		
		// Mouse y-axis affect depends on modifiers.
		if(pitchModifierActive)
		{
			pitchSpeed += mouseDelta.y * kRotationSpeed;
		}
		else if(panModifierActive)
		{
			verticalSpeed += mouseDelta.y * kSpeed;
		}
		else
		{
			forwardSpeed += mouseDelta.y * kSpeed;
		}
		
		// Mouse x-axis affect also depends on modifiers.
		// Note that pitch modifier and no modifier do the same thing!
		if(pitchModifierActive)
		{
			turnSpeed += mouseDelta.x * kRotationSpeed;
		}
		else if(panModifierActive)
		{
			strafeSpeed += mouseDelta.x * kSpeed;
		}
		else
		{
			turnSpeed += mouseDelta.x * kRotationSpeed;
		}
	}
	
	// Alt keys just increase all speeds!
	if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LALT) ||
	   Services::GetInput()->IsKeyPressed(SDL_SCANCODE_RALT))
	{
		forwardSpeed *= kFastSpeedMultiplier;
		strafeSpeed *= kFastSpeedMultiplier;
		turnSpeed *= kFastSpeedMultiplier;
		pitchSpeed *= kFastSpeedMultiplier;
		verticalSpeed *= kFastSpeedMultiplier;
	}
	
	// Apply forward movement.
	// Forward movement should be only on X/Z plane.
	Vector3 forward = GetForward();
	forward.y = 0.0f;
	forward.Normalize();
	GetTransform()->Translate(forward * forwardSpeed * deltaTime);
	
	// Apply strafe movement.
	GetTransform()->Translate(GetRight() * strafeSpeed * deltaTime);
	
	// Apply turn movement.
	GetTransform()->Rotate(Vector3::UnitY, turnSpeed * deltaTime, Transform::Space::World);
	
	// Apply pitch movement.
	GetTransform()->Rotate(GetRight(), -pitchSpeed * deltaTime, Transform::Space::World);
	
	// Apply height.
	mHeight += verticalSpeed * deltaTime;
	
	// Raycast to the ground and always maintain a desired height.
	//TODO: This does not apply when camera boundaries are disabled!
	//TODO: Doesn't apply when middle mouse button is held???
	Scene* scene = GEngine::inst->GetScene();
	if(scene != nullptr)
	{
		Vector3 pos = GetPosition();
		float floorY = scene->GetFloorY(pos);
		pos.SetY(floorY + mHeight);
		SetPosition(pos);
	}
	
	// For debugging, output camera position on key press.
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_P))
	{
		std::cout << GetPosition() << std::endl;
	}
	
	// Handle hovering and clicking on scene objects.
	//TODO: Original game seems to ONLY check this when the mouse cursor moves or is clicked (in other words, on input).
	//TODO: Maybe we should do that too?
	if(mCamera != nullptr && !Services::GetInput()->MouseLocked())
	{
		// Only allow scene interaction if pointer isn't over a UI widget.
		if(!UICanvas::DidWidgetEatInput())
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
	}
	
	// Clear camera lock if left mouse is not pressed.
	// Do this AFTER interact check to avoid interacting with things when exiting mouse locked movement mode.
	if(!leftMousePressed)
	{
		Services::GetInput()->UnlockMouse();
	}
}
