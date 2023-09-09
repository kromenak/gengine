#include "GameCamera.h"

#include "ActionManager.h"
#include "AudioListener.h"
#include "Camera.h"
#include "Collisions.h"
#include "CursorManager.h"
#include "Debug.h"
#include "GKActor.h"
#include "GKObject.h"
#include "InventoryManager.h"
#include "Mesh.h"
#include "Model.h"
#include "OptionBar.h"
#include "Ray.h"
#include "SaveManager.h"
#include "SceneManager.h"
#include "Sphere.h"
#include "StringUtil.h"
#include "Triangle.h"
#include "VerbManager.h"
#include "UICanvas.h"

/*static*/ void GameCamera::SetCameraGlideEnabled(bool enabled)
{
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_CAMERA_GLIDE, enabled);
}

/*static*/ bool GameCamera::IsCameraGlideEnabled()
{
    return gSaveManager.GetPrefs()->GetBool(PREFS_ENGINE, PREF_CAMERA_GLIDE, true);
}

/*static*/ void GameCamera::SetCinematicsEnabled(bool enabled)
{
    gSaveManager.GetPrefs()->Set(PREFS_ENGINE, PREF_CINEMATICS, enabled);
}

/*static*/ bool GameCamera::AreCinematicsEnabled()
{
    return gSaveManager.GetPrefs()->GetBool(PREFS_ENGINE, PREF_CINEMATICS, true);
}

GameCamera::GameCamera() : Actor("Camera")
{
    mCamera = AddComponent<Camera>();
    AddComponent<AudioListener>();
    
    // Create option bar.
    mOptionBar = new OptionBar();
}

void GameCamera::RemoveBounds(Model* model)
{
    auto it = std::find(mBoundsModels.begin(), mBoundsModels.end(), model);
    if(it != mBoundsModels.end())
    {
        mBoundsModels.erase(it);
    }
}

void GameCamera::SetAngle(const Vector2& angle)
{
	SetAngle(angle.x, angle.y);
}

void GameCamera::SetAngle(float yaw, float pitch)
{
	SetRotation(Quaternion(Vector3::UnitY, yaw) * Quaternion(Vector3::UnitX, pitch));
}

void GameCamera::Glide(const Vector3& position, const Vector2& angle, std::function<void()> callback)
{
    mGliding = true;
    mEndGlideCallback = callback;

    mGlidePosition = position;
    mGlideRotation = Quaternion(Vector3::UnitY, angle.x) * Quaternion(Vector3::UnitX, angle.y);

    mGlideStartPos = GetTransform()->GetPosition();
    mGlideStartRot = GetTransform()->GetRotation();

    mGlideTimer = 0.0f;
    
    // A glide request would cancel any inspect that was occurring.
    mInspectNoun.clear();
}

void GameCamera::Inspect(const std::string& noun, const Vector3& position, const Vector2& angle, std::function<void()> callback)
{
    // Save spot we were at when we started to inspect.
    mInspectStartPos = GetTransform()->GetPosition();
    mInspectStartRot = GetTransform()->GetRotation();

    // Glide to the inspect position/angle.
    Glide(position, angle, callback);

    // We ARE inspecting something. (Do this after calling Glide b/c that clears this value).
    mInspectNoun = noun;
}

void GameCamera::Uninspect(std::function<void()> callback)
{
    if(!mInspectNoun.empty())
    {
        Glide(mInspectStartPos, Vector2::Zero, callback);
        mGlideRotation = mInspectStartRot; //HACK: Just overwrite with saved rotation here.

        mInspectNoun.clear();
    }
}

void GameCamera::SaveFov()
{
    mSavedCameraFOV = mCamera->GetCameraFovRadians();
}

void GameCamera::RestoreFov()
{
    mCamera->SetCameraFovRadians(mSavedCameraFOV);
}

void GameCamera::OnUpdate(float deltaTime)
{
    /*
    {
        static Vector3 sphereStartPos(50.0f, 40.0f, 150.0f);
        static Vector3 sphereEndPos(100.0f, 80.0f, 300.0f);
        static Vector3 sphereCurrentPos = sphereStartPos;

        if(!mBoundsModels.empty() && mBoundsEnabled)
        {
             // Draw spheres for debug visualization.
            Sphere sphere(sphereStartPos, kCameraColliderRadius);
            Sphere sphereEnd(sphereEndPos, kCameraColliderRadius);

            Debug::DrawSphere(sphere, Color32::Blue);
            Debug::DrawSphere(sphereEnd, Color32::Red);
            Debug::DrawLine(sphereStartPos, sphereEndPos, Color32::White);
            
            // Test collision resolution code.
            //Vector3 collidePos = ResolveCollisions(sphereCurrentPos, sphereEndPos - sphereCurrentPos);
            //Sphere sphereCurrent(collidePos, kCameraColliderRadius);
            //Debug::DrawSphere(sphereCurrent, Color32::Green);

            Vector3 moveOffset = Vector3::Normalize(sphereEndPos - sphereStartPos) * 5.0f * deltaTime;
            sphereCurrentPos = ResolveCollisions(sphereCurrentPos, moveOffset);
            Sphere sphereCurrent(sphereCurrentPos, kCameraColliderRadius);
            Debug::DrawSphere(sphereCurrent, Color32::Green);
        }
    }
    */
    
    // Perform scene-only updates (camera movement, click-to-interact), if scene is active and no action is playing.
    if(mSceneActive)
    {
        SceneUpdate(deltaTime);
    }

    // Show options on right-click. This works even if an action is playing.
    if(!mUsedMouseInputsForMouseLock)
    {
        if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Right))
        {
            mOptionBar->Show();
        }
    }

    // If no mouse button is pressed, we can clear the mouse locked with current inputs flag.
    if(mUsedMouseInputsForMouseLock)
    {
        if(!gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left) &&
           !gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Right))
        {
            mUsedMouseInputsForMouseLock = false;
        }
    }
    
    // BELOW HERE: logic for player's keyboard shortcuts.
    // Keyboard shortcut keys are only available if text input is not active.
    if(!gInputManager.IsTextInput())
    {
        // Some keyboard shortcuts are only available when an action is not playing.
        bool actionPlaying = gActionManager.IsActionPlaying();
        if(!actionPlaying)
        {
            // If 'I' is pressed, toggle inventory.
            if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_I))
            {
                // If the scene is active, show the inventory.
                // If the inventory is showing, hide it.
                if(mSceneActive)
                {
                    gInventoryManager.ShowInventory();
                }
                else if(gInventoryManager.IsInventoryShowing())
                {
                    gInventoryManager.HideInventory();
                }
            }
        }
        
        // If 'P' is pressed, this toggles game pause. Works even if action is ongoing.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_P))
        {
            //TODO: implement pause!
            std::cout << "Pause!" << std::endl;

            // For debugging...
            std::cout << "Pos: " << GetPosition() << ", Heading: " << Heading::FromQuaternion(GetRotation()) << std::endl;
            
        }

        // Pressing escape acts as a "skip" or "cancel" action, depending on current state of the game.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
        {
            gSceneManager.GetScene()->SkipCurrentAction();
        }
    }
}

void GameCamera::SceneUpdate(float deltaTime)
{
    // Handle camera glide behavior.
    if(mGliding)
    {
        float t = Math::Clamp(mGlideTimer / kGlideDuration, 0.0f, 1.0f);
        if(!IsCameraGlideEnabled())
        {
            t = 1.0f;
        }
        
        // Interpolate towards desired position.
        GetTransform()->SetPosition(Vector3::Lerp(mGlideStartPos, mGlidePosition, t));

        // Rotate towards desired rotation.
        Quaternion newRotation;
        Quaternion::Slerp(newRotation, mGlideStartRot, mGlideRotation, t);
        GetTransform()->SetRotation(newRotation);

        // Increment timer.
        mGlideTimer += deltaTime;

        // If reached end of glide, stop.
        if(t >= 1.0f)
        {
            mGliding = false;
            if(mEndGlideCallback)
            {
                mEndGlideCallback();
            }
        }
        return;
    }

    // It's possible our height was changed due to a script moving the camera.
    // Make sure height is correct before we do our updates.
    float startFloorY = gSceneManager.GetScene()->GetFloorY(GetPosition());
    mHeight = GetPosition().y - startFloorY;

    // Update camera movement/rotation.
    SceneUpdateMovement(deltaTime);
    
    // Raycast to the ground and always maintain a desired height.
    //TODO: This does not apply when camera boundaries are disabled!
    //TODO: Doesn't apply when middle mouse button is held???
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        Vector3 pos = GetPosition();
        float floorY = scene->GetFloorY(pos);
        pos.y = floorY + mHeight;
        SetPosition(pos);
    }

    // Check for scene interaction based on mouse pointer position and mouse button presses.
    SceneUpdateInteract(deltaTime);
    
    // Clear camera lock if left mouse is not pressed.
    // Do this AFTER interact check to avoid interacting with things when exiting mouse locked movement mode.
    if(!gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        gInputManager.UnlockMouse();
    }
}

void GameCamera::SceneUpdateMovement(float deltaTime)
{
    // No movements are allowed during forced cinematic mode.
    if(mForcedCinematicMode) { return; }

    // Movement is not allowed during actions UNLESS cinematic camera is disabled.
    if(gActionManager.IsActionPlaying() && AreCinematicsEnabled())
    {
        return;
    }

    // We don't move/turn unless some input causes it.
    float forwardSpeed = 0.0f;
    float strafeSpeed = 0.0f;
    float turnSpeed = 0.0f;
    float pitchSpeed = 0.0f;
    float verticalSpeed = 0.0f;

    // Pan/Pan modifiers are activated with CTRL/SHIFT keys.
    // These work EVEN IF text input is active.
    bool panModifierActive = gInputManager.IsKeyPressed(SDL_SCANCODE_LCTRL) ||
        gInputManager.IsKeyPressed(SDL_SCANCODE_RCTRL);;
    bool pitchModifierActive = gInputManager.IsKeyPressed(SDL_SCANCODE_LSHIFT) ||
        gInputManager.IsKeyPressed(SDL_SCANCODE_RSHIFT);

    // W/S/A/D and Up/Down/Left/Right only work if text input isn't stealing input.
    if(!gInputManager.IsTextInput())
    {
        if(gInputManager.IsKeyPressed(SDL_SCANCODE_W) ||
           gInputManager.IsKeyPressed(SDL_SCANCODE_UP))
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
        else if(gInputManager.IsKeyPressed(SDL_SCANCODE_S) ||
                gInputManager.IsKeyPressed(SDL_SCANCODE_DOWN))
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

        if(gInputManager.IsKeyPressed(SDL_SCANCODE_D) ||
           gInputManager.IsKeyPressed(SDL_SCANCODE_RIGHT))
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
        if(gInputManager.IsKeyPressed(SDL_SCANCODE_A) ||
           gInputManager.IsKeyPressed(SDL_SCANCODE_LEFT))
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

        // Handle spacebar input, which resets camera pitch/height.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_SPACE))
        {
            // Reset pitch by discarding all rotation about the right axis, while keeping all else.
            Quaternion current = GetTransform()->GetRotation();
            GetTransform()->SetRotation(current.Discard(GetRight()));

            // Resetting height is a bit more straightforward.
            mHeight = kDefaultHeight;
        }
    }

    // If left mouse button is held down, mouse movement contributes to camera movement.
    bool leftMousePressed = gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left);
    if(leftMousePressed && !UICanvas::DidWidgetEatInput())
    {
        // Track click start position for turning on mouse-based camera movement.
        // To avoid mistakenly enabling this, you must move the mouse some distance before it enables.
        Vector2 mousePosition = gInputManager.GetMousePosition();
        if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
        {
            mClickStartPos = mousePosition;
        }
        else if((mClickStartPos - mousePosition).GetLengthSq() > 5 * 5)
        {
            // Moved the mouse far enough, so enable mouse lock.
            mUsedMouseInputsForMouseLock = true;
            gInputManager.LockMouse();
        }

        // Do mouse-based movements if mouse lock is active.
        if(gInputManager.MouseLocked())
        {
            // Pan modifier also activates if right mouse button is pressed.
            panModifierActive |= gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Right);

            // Mouse delta is in pixels.
            // We normalize this by estimating "max pixel movement" per frame.
            Vector2 mouseDelta = gInputManager.GetMouseDelta();
            mouseDelta /= kMouseRangePixels;

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
    }

    // Alt keys just increase all speeds!
    if(gInputManager.IsKeyPressed(SDL_SCANCODE_LALT) ||
       gInputManager.IsKeyPressed(SDL_SCANCODE_RALT))
    {
        forwardSpeed *= kFastSpeedMultiplier;
        strafeSpeed *= kFastSpeedMultiplier;
        turnSpeed *= kFastSpeedMultiplier;
        pitchSpeed *= kFastSpeedMultiplier;
        verticalSpeed *= kFastSpeedMultiplier;
    }

    // For forward movement, we want to disregard any y-facing; just move on X/Z plane.
    Vector3 forward = GetForward();
    forward.y = 0.0f;
    forward.Normalize();

    // Calculate desired position based on all speeds.
    // We may not actually move to this exact position, due to collision.
    Vector3 position = GetPosition();
    position += forward * forwardSpeed * deltaTime;
    position += GetRight() * strafeSpeed * deltaTime;

    // Calculate new desired height and apply that to position y.
    float height = mHeight + verticalSpeed * deltaTime;
    float floorY = gSceneManager.GetScene()->GetFloorY(position);
    position.y = floorY + height;

    // Determine offset from start to end position.
    Vector3 moveOffset = position - GetPosition();

    // Perform collision checks and resolutions.
    position = ResolveCollisions(GetPosition(), moveOffset);

    // Set position after resolving collisions.
    GetTransform()->SetPosition(position);

    // Height may also be affected by collision. After resolving,
    // we can see if our height changed and save it.
    float newFloorY = gSceneManager.GetScene()->GetFloorY(position);
    float heightForReal = position.y - newFloorY;
    mHeight = heightForReal;

    // Apply turn movement.
    GetTransform()->Rotate(Vector3::UnitY, turnSpeed * deltaTime, Transform::Space::World);

    // Apply pitch movement.
    GetTransform()->Rotate(GetRight(), -pitchSpeed * deltaTime, Transform::Space::World);

    // If we were inspecting something, but we move the camera, that "breaks" the inspect state.
    if(moveOffset.GetLengthSq() > 1)
    {
        mInspectNoun.clear();
    }
}

void GameCamera::SceneUpdateInteract(float deltaTime)
{
    // Never allowed to interact while an action is playing.
    if(gActionManager.IsActionPlaying())
    {
        return;
    }

    // Handle hovering and clicking on scene objects.
    //TODO: Original game seems to ONLY check this when the mouse cursor moves or is clicked (in other words, on input).
    //TODO: Maybe we should do that too?
    if(!gInputManager.MouseLocked())
    {
        // Only allow scene interaction if pointer isn't over a UI widget.
        if(!UICanvas::DidWidgetEatInput())
        {
            // Calculate mouse click ray.
            Vector2 mousePos = gInputManager.GetMousePosition();
            Vector3 worldPos = mCamera->ScreenToWorldPoint(mousePos, 0.0f);
            Vector3 worldPos2 = mCamera->ScreenToWorldPoint(mousePos, 1.0f);
            Vector3 dir = (worldPos2 - worldPos).Normalize();
            Ray ray(worldPos, dir);

            // Cast into the scene to see if we're over an interactive object.
            SceneCastResult result = gSceneManager.GetScene()->Raycast(ray, true);

            // If we can interact with whatever we are pointing at, highlight the cursor.
            // Note we call "UseHighlightCursor" when start hovering OR we switch hover to new object.
            // This toggles red/blue highlight.
            GKObject* hovering = result.hitObject;
            if(hovering != nullptr)
            {
                if(!StringUtil::EqualsIgnoreCase(hovering->GetNoun(), mLastHoveredNoun))
                {
                    // See if the hovered item has a custom verb with an associated custom cursor.
                    Cursor* customCursor = nullptr;
                    if(!hovering->GetVerb().empty())
                    {
                        customCursor = gVerbManager.GetVerbIcon(hovering->GetVerb()).cursor;
                    }

                    // Set cursor appropriately.
                    if(customCursor != nullptr)
                    {
                        gCursorManager.UseCustomCursor(customCursor);
                    }
                    else
                    {
                        gCursorManager.UseHighlightCursor();
                    }
                    mLastHoveredNoun = hovering->GetNoun();
                }
            }
            else
            {
                gCursorManager.UseDefaultCursor();
                mLastHoveredNoun.clear();
            }

            // If left mouse button is released, try to interact with whatever it is over.
            // Need to do this, even if canInteract==false, because floor can be clicked to move around.
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                gSceneManager.GetScene()->Interact(ray, hovering);
            }
        }
        else
        {
            mLastHoveredNoun.clear();
        }
    }
}

Vector3 GameCamera::ResolveCollisions(const Vector3& startPosition, const Vector3& moveOffset)
{
	// No bounds model = no collision.
	// Bounds may also be purposely disabled for debugging purposes.
	if(mBoundsModels.empty() || !mBoundsEnabled || gInputManager.IsKeyPressed(SDL_SCANCODE_SPACE))
    {
        return startPosition + moveOffset;
    }

    // No movement means no need to resolve collisions.
    if(Math::IsZero(moveOffset.GetLengthSq()))
    {
        return startPosition;
    }

    // Ok, we begin at the start position.
    Vector3 currentPosition = startPosition;
    Vector3 currentMoveOffset = moveOffset;

    // We need to check for collisions with any triangles along the path from camera's start to end point this frame.
    // If a collision occurs, the velocity is "redirected" along the collided surface, and collisions must then be checked again (i.e. iteratively/recursively).
    // So, that's why we must loop here until the velocity gets close to zero (or we detect no more collisions).
    int passCount = 0;
    while(currentMoveOffset.GetLengthSq() > 0.0f && passCount < 2)
    {
        ++passCount;

        // Assume, by default, we haven't collided with anything.
        // "t" represents % of velocity we will move, so a default of 1 means "didn't collide with anything => move full amount".
        bool collided = false;
        float smallestT = 1.0f;
        Vector3 collisionNormal;
        Triangle collideTriangle;
        
        // Create sphere at current position.
        Sphere sphere(currentPosition, kCameraColliderRadius);

        /*
        {
            Vector3 p0 = Vector3::UnitZ * 200.0f + Vector3::UnitY * 10.0f;
            Vector3 p1 = p0 + (Vector3::UnitY * 100.0f);
            Vector3 p2 = p0 + (Vector3::UnitX * 250.0f);
            Vector3 p3 = p2 + (Vector3::UnitY * 100.0f);

            Triangle t1(p0, p1, p2);
            Debug::DrawTriangle(t1, Color32::Yellow);
            Debug::DrawLine(t1.GetCenter(), t1.GetCenter() + t1.GetNormal() * 10.0f, Color32::Yellow);

            //Triangle t2(p1, p3, p2);
            //Debug::DrawTriangle(t2, Color32::Yellow);
            //Debug::DrawLine(t2.GetCenter(), t2.GetCenter() + t2.GetNormal() * 10.0f, Color32::Yellow);
            std::vector<Triangle> tris { t1 }; // t2 };
            for(auto& tri : tris)
            {
                // Check collision and record the t/normal if it's smaller than any previously discovered one.
                float sphereT = 0.0f;
                Vector3 normal;
                if(Collide::SphereTriangle(sphere, tri, currentMoveOffset, sphereT, normal))
                {
                    if(sphereT < smallestT)
                    {
                        collided = true;
                        smallestT = sphereT;
                        collisionNormal = normal;
                        collideTriangle = tri;
                    }
                }
            }
        }
        */

        // Check collision against each individual triangle of each bounds model.
        for(auto& model : mBoundsModels)
        {
            auto& meshes = model->GetMeshes();
            for(auto& mesh : meshes)
            {
                // Bounds model is positioned at (0,0,0) in world space (so no need to multiply local to world...it's identity).
                Matrix4 meshToWorld = mesh->GetMeshToLocalMatrix();

                auto submeshes = mesh->GetSubmeshes();
                for(auto& submesh : submeshes)
                {
                    int triangleCount = submesh->GetTriangleCount();
                    for(int i = 0; i < triangleCount; ++i)
                    {
                        Vector3 p0, p1, p2;
                        if(submesh->GetTriangle(i, p0, p1, p2))
                        {
                            // Transform triangle to world space.
                            p0 = meshToWorld.TransformPoint(p0);
                            p1 = meshToWorld.TransformPoint(p1);
                            p2 = meshToWorld.TransformPoint(p2);
                            Triangle triangle(p0, p1, p2);

                            // Check collision and record the t/normal if it's smaller than any previously discovered one.
                            float sphereT = 0.0f;
                            Vector3 normal;
                            if(Collide::SphereTriangle(sphere, triangle, currentMoveOffset, sphereT, normal))
                            {
                                // If a triangle reports a negative-t collision, it means we are already intersecting it.
                                // We better be actively intersecting in this case.
                                //TODO: Probably Collide::SphereTriangle should handle this internally?
                                Vector3 intersectPoint;
                                if(sphereT < 0.0f && !Intersect::TestSphereTriangle(sphere, triangle, intersectPoint))
                                {
                                    continue;
                                }

                                if(sphereT < smallestT)
                                {
                                    collided = true;
                                    smallestT = sphereT;
                                    collisionNormal = normal;
                                    collideTriangle = triangle;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Move sphere.
        Vector3 moveFrom = currentPosition;
        currentPosition += currentMoveOffset * smallestT;
        //Debug::DrawLine(moveFrom, currentPosition, Color32::Cyan);
        //std::cout << "Moved from " << moveFrom << " to " << currentPosition << "; offset=" << currentMoveOffset << ", t=" << smallestT << std::endl;

        // If we didn't collide with anything, we are done.
        if(!collided)
        {
            break;
        }

        //Debug::DrawTriangle(collideTriangle, Color32::Yellow);
        //Debug::DrawLine(collideTriangle.GetCenter(), collideTriangle.GetCenter() + collideTriangle.GetNormal() * 5.0f, Color32::Yellow);

        // We DID collide with something, so the camera's movement was cut short. This means only a portion of its velocity was utilized.
        // If we just stopped here, the camera's movement would be quite jerky and hard to control.
        // To have it "glide" along walls and obstacles, we must "redirect" remaining velocity in a new direction and then do collision checks again.

        // Calculate a plane representing the surface of the triangle we collided with.
        Plane tangentPlane(collisionNormal, currentPosition);
        //Debug::DrawPlane(tangentPlane, currentPosition, Color32::Magenta);

        // Project velocity onto tangent plane, which then gives us a new velocity.
        // The "size" of the velocity is dictated by how much we were facing the wall - head on gives small velocity, at a steep angle gives larger velocity.
        // But the velocity *should* always grow smaller each iteration.
        Vector3 tangentPoint = tangentPlane.GetClosestPoint(moveFrom + currentMoveOffset);

        //Debug::DrawLine(moveFrom, moveFrom + currentMoveOffset, Color32::Magenta, 10.0f);
        //Debug::DrawLine(moveFrom + currentMoveOffset, tangentPoint, Color32::Blue, 10.0f);
        //Debug::DrawLine(currentPosition, tangentPoint, Color32::White, 10.0f);

        currentMoveOffset = tangentPoint - currentPosition;
        //Debug::DrawLine(currentPosition, currentPosition + currentMoveOffset, Color32::Red);
    }
    
    // Return whatever our final position was!
    return currentPosition;
}
