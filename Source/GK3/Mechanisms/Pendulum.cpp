#include "Pendulum.h"

#include "ActionManager.h"
#include "Animator.h"
#include "AssetManager.h"
#include "CursorManager.h"
#include "Debug.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "InputManager.h"
#include "Interpolate.h"
#include "MeshRenderer.h"
#include "PersistState.h"
#include "SceneManager.h"
#include "Timers.h"

Pendulum::Pendulum() : Actor("Pendulum Puzzle")
{
    // Cache pendulum actors, use the normal one by default.
    mNormalPendulumActor = gSceneManager.GetScene()->GetSceneObjectByModelName("te3_pendulum_center_code");
    mGabePendulumActor = gSceneManager.GetScene()->GetSceneObjectByModelName("te3__pendulum_gabe");
    mPendulumActor = mNormalPendulumActor;

    // Calculate pivot point based on initial actor position.
    // We're assuming the pendulum is initially at the bottom of the oscillation arc. So pivot point is straight up.
    mPendulumPivotPoint = mPendulumActor->GetPosition() + Vector3::UnitY * kPendulumArmRadius;

    // Cache all the platform actors for efficient access.
    std::string platformModelName = "te3_r01";
    for(int i = 1; i <= kPlatformCount; ++i)
    {
        platformModelName[5] = '0' + static_cast<char>(i / 10);
        platformModelName[6] = '0' + static_cast<char>(i % 10);
        mPlatforms[i - 1] = gSceneManager.GetScene()->GetSceneObjectByModelName(platformModelName);
    }

    // Cache the altar for later use.
    mAltarActor = gSceneManager.GetScene()->GetSceneObjectByModelName("te3_hpaltar");

    // Cache ego-related values as well.
    mGabeStartPosition = gSceneManager.GetScene()->GetPosition("LookOut")->position;
    mGabeActor = gSceneManager.GetScene()->GetEgo();
    mGabeMicActor = gSceneManager.GetScene()->GetSceneObjectByModelName("gabmic");
    mGabeBloodActor = gSceneManager.GetScene()->GetSceneObjectByModelName("groundblood");

    // Load animations used during this sequence.
    mIdleAnim = gAssetManager.LoadAnimation("gabBreathMic", AssetScope::Scene);

    mJumpForwardStartAnim = gAssetManager.LoadAnimation("GABTE3JUMPFS", AssetScope::Scene);
    mJumpForwardEndAnim = gAssetManager.LoadAnimation("GABTE3JUMPFE", AssetScope::Scene);

    mJumpLeftStartAnim = gAssetManager.LoadAnimation("GABTE3JUMPLS", AssetScope::Scene);
    mJumpLeftMiddleAnim = gAssetManager.LoadAnimation("GABTE3JUMPLM", AssetScope::Scene);
    mJumpLeftEndAnim = gAssetManager.LoadAnimation("GABTE3JUMPLE", AssetScope::Scene);

    mJumpRightStartAnim = gAssetManager.LoadAnimation("GABTE3JUMPRS", AssetScope::Scene);
    mJumpRightMiddleAnim = gAssetManager.LoadAnimation("GABTE3JUMPRM", AssetScope::Scene);
    mJumpRightEndAnim = gAssetManager.LoadAnimation("GABTE3JUMPRE", AssetScope::Scene);

    // Idle anim loops.
    mIdleAnimParams.animation = mIdleAnim;
    mIdleAnimParams.loop = true;

    // Jump anim can move the actor.
    mJumpAnimParams.allowMove = true;

    // Cache the grab cursor.
    mGrabCursor = gAssetManager.LoadCursor("C_GRAB.CUR", AssetScope::Scene);

    // There's a PENDULUM.TXT that these values can be read from.
    // But frankly, it's so simple, I'm not going to bother at the moment.
    float platformRevolutionDuration = 50.0f;
    int platformsPerPendulumCycle = 6;
    mPendulumMaxAngle = Math::ToRadians(24.5f);

    // The amount of time it takes for the pendulum to complete one cycle is derived in a truly deranged way.
    // We know how many platforms should pass the pendulum between cycles.
    // We know the rotation speeds of the platforms. So...let's figure this out.
    //
    // The platforms rotate in a circle over the course of 50 seconds. So, the radians of movement per second are 2pi/50.
    mPlatformRotationSpeed = Math::k2Pi / platformRevolutionDuration;

    // The amount of time it takes for a single platform to pass the pendulum is then the radians per platform divided by radians per second.
    // The pendulum is supposed to complete a cycle every 6 block moves, so we can now calculate how much time that should be.
    float platformDuration = kRadiansPerPlatform / mPlatformRotationSpeed;
    mPendulumCycleDuration = platformDuration * platformsPerPendulumCycle;

    // Set pendulum initial rotation based on derived values.
    SetPendulumActorRotation(mPendulumMaxAngle);

    // Because this Pendulum sequence is so complex, all animations are controlled in this class, as opposed to via the fidget or walk systems.
    // Start by looping Gabe's idle animation.
    gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);

    // The floor is kind of weird in this level (only the center altar area).
    // To avoid some weirdness with the camera height-adjusting, ignore height for camera in this scene.
    gSceneManager.GetScene()->GetCamera()->SetIgnoreFloor(true);
}

void Pendulum::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mPendulumCycleTimer));
    ps.Xfer(PERSIST_VAR(mPlatformRotation));
    ps.Xfer(PERSIST_VAR(mGabePlatformIndex));
    ps.Xfer<GabeState, int>(PERSIST_VAR(mGabeState));

    // When loading, we need to do a pendulum update to ensure a) proper rotation, and b) using right pendulum actor based on Gabe state.
    if(ps.IsLoading())
    {
        UpdatePendulum(0.0f);

        // Due to the dynamic nature of this scene, Gabe's jumps aren't actions/cutscenes, so you CAN save during them.
        // However, we aren't saving jump state! If we detect a save during a jump, just put Gabe back on the last platform he was on.
        if(mGabeState == GabeState::Jumping)
        {
            mGabeState = GabeState::OnPlatform;
        }

        // The constructor starts Gabe's idle animation. But if Gabe's in a situation where that's not needed after loading, it can conflict.
        // For example, if Gabe's on the altar, he has a specific idle GAS he's using, so we want to cancel the one we set.
        if(mGabeState != GabeState::InEntryway && mGabeState != GabeState::OnPlatform)
        {
            gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim, true);
        }
    }
}

void Pendulum::OnUpdate(float deltaTime)
{
    UpdateGabe(deltaTime);
    UpdatePendulum(deltaTime);
    UpdatePlatforms(deltaTime);
}

void Pendulum::UpdatePendulum(float deltaTime)
{
    // Increase current cycle timer.
    // If we exceed the duration, subtract it to get back within a valid range.
    mPendulumCycleTimer += deltaTime;
    if(mPendulumCycleTimer >= mPendulumCycleDuration)
    {
        mPendulumCycleTimer -= mPendulumCycleDuration;
    }

    // Update the actor used for the pendulum.
    // The game uses two versions (one with Gabe, one without) at different parts of this puzzle.
    if(mGabeState == GabeState::AtAltar)
    {
        // Turn off both pendulums when at altar.
        // A third pendulum model is used at the altar, but it's controlled from Sheepscript entirely.
        mGabePendulumActor->SetActive(false);
        mNormalPendulumActor->SetActive(false);
    }
    else if(mGabeState == GabeState::OnPendulum)
    {
        UseGabePendulum();
    }
    else
    {
        UseNormalPendulum();
    }

    // Update the angle used by the in-game 3D object.
    float angle = GetPendulumAngle();
    SetPendulumActorRotation(angle);

    // If the pendulum is near the wall slots on either the left or right sides of the room, we need to check if Gabe is going to die.
    // The "5 degrees" here is not from the original game - I just eyeballed it.
    float maxAngleDiff = Math::Abs(mPendulumMaxAngle - Math::Abs(angle));
    bool inDangerZone = maxAngleDiff < Math::ToRadians(8.0f);
    if(inDangerZone)
    {
        bool onLeftSide = angle > 0.0f;
        mPendulumState = onLeftSide ? PendulumState::InDangerZoneLeft : PendulumState::InDangerZoneRight;

        // If Ego is on a platform, and that platform is in front of left/right slots when the blade is there...Ego is dead!
        if(mGabeState == GabeState::OnPlatform)
        {
            if((onLeftSide && GetIndexOfPlatformAtLeftPendulumSlot() == mGabePlatformIndex) ||
               (!onLeftSide && GetIndexOfPlatformAtRightPendulumSlot() == mGabePlatformIndex))
            {
                OnPendulumPlatformDeath(onLeftSide, mPendulumCycleTimer > (mPendulumCycleDuration * 0.5f));
            }
        }
    }
    else
    {
        mPendulumState = PendulumState::Swinging;
    }
}

void Pendulum::SetPendulumActorRotation(float angle)
{
    // We're assuming the pendulum rotates about the z-axis.
    Quaternion rot(Vector3::UnitZ, angle);

    // Figure out which direction corresponds to this angle, assuming straight down is angle of 0.0f.
    Vector3 rotated = rot.Rotate(-Vector3::UnitY);
    //Debug::DrawLine(mPivotPoint, mPivotPoint + rotated * kArmRadius, Color32::Red, 60.0f);

    // Rotate the pendulum to that rotation.
    mPendulumActor->GetTransform()->SetRotation(rot);

    // For whatever reason, the pendulum model doesn't have its pivot at the pivot point.
    // To compensate for this, we also need to position the model at a corrected position, calculated based on the arm rotation and length.
    mPendulumActor->GetTransform()->SetPosition(mPendulumPivotPoint + rotated * kPendulumArmRadius);
}

float Pendulum::GetPendulumAngle()
{
    // Use cubic interpolation to calculate the pendulum's angle between min and max along the desired duration.
    // The duration represents a full swing out AND back, so we need to account for switching direction halfway through the duration.
    float angle = 0.0f;
    float halfCycleDuration = mPendulumCycleDuration * 0.5f;
    if(mPendulumCycleTimer <= halfCycleDuration)
    {
        angle = Interpolate::CubicInOut(mPendulumMaxAngle, -mPendulumMaxAngle, mPendulumCycleTimer / halfCycleDuration);
    }
    else
    {
        angle = Interpolate::CubicInOut(-mPendulumMaxAngle, mPendulumMaxAngle, (mPendulumCycleTimer - halfCycleDuration) / halfCycleDuration);
    }
    return angle;
}

void Pendulum::UseNormalPendulum()
{
    // Activate normal pendulum.
    mGabePendulumActor->SetActive(false);
    mNormalPendulumActor->SetActive(true);
    mPendulumActor = mNormalPendulumActor;

    // Turn on Gabe and his mic.
    mGabeActor->SetActive(true);
    mGabeMicActor->SetActive(true);

    // Make sure pendulum is at correct rotation after swap.
    SetPendulumActorRotation(GetPendulumAngle());
}

void Pendulum::UseGabePendulum()
{
    // Activate Gabe pendulum.
    mGabePendulumActor->SetActive(true);
    mNormalPendulumActor->SetActive(false);
    mPendulumActor = mGabePendulumActor;

     // Hide Gabe's normal model and stop animating it.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);
    mGabeActor->SetActive(false);
    mGabeMicActor->SetActive(false);

    // Make sure pendulum is at correct rotation after swap.
    SetPendulumActorRotation(GetPendulumAngle());
}

void Pendulum::UpdatePlatforms(float deltaTime)
{
    // Update rotation.
    mPlatformRotation += mPlatformRotationSpeed * deltaTime;

    // Apply rotation to all platforms.
    Heading heading = Heading::FromRadians(mPlatformRotation);
    for(GKObject* platform : mPlatforms)
    {
        platform->SetHeading(heading);
    }
}

int Pendulum::GetIndexOfPlatformAtEntryway()
{
    // We need to know which platform is in front of the entryway.
    // This could be quite hairy, but fortunately, we know two things:
    // 1) Platform 23 is in front of the entryway when rotation is zero.
    // 2) Each platform's "length" corresponds to a rotation amount about the room (kRadiansPerPlatform).
    // Given these two things, we can calculate *exactly* the platform in front of the entryway pretty easily!

    // Convert to radians between 0 and 2Pi. Converting to/from a heading takes care of this.
    float rotation = Heading::FromRadians(mPlatformRotation).ToRadians();

    // At a rotation 0 zero, no platforms have moved past the entry.
    // But every kRadiansPerPlatform radians, one does. So we can calculate how many platforms have moved past the entryway.
    int platformsPastEntry = static_cast<int>(rotation / kRadiansPerPlatform);

    // The index of the platform AT the entryway is then the last index minus the number that have rotated past.
    return (kPlatformCount - 1) - platformsPastEntry;
}

int Pendulum::GetIndexOfPlatformAtLeftPendulumSlot()
{
    // This returns the index of the platform that's currently in front of the left slot that the pendulum swings in to.
    // When I say "left", I mean from the perspective of Gabe when he enters the room. The first slot Gabe encounters as he rides around the room.

    // A tricky way to calculate this: it's always 6 platforms ahead of the one at the entryway.
    // So just use that, with wraparound.
    return (GetIndexOfPlatformAtEntryway() + 6) % kPlatformCount;
}

int Pendulum::GetIndexOfPlatformAtRightPendulumSlot()
{
    // Same as above, but for the right slot (from Gabe's perspective when he enters the room).
    // In this case, it's 6 *behind*, with wraparound.
    int index = GetIndexOfPlatformAtEntryway() - 6;
    if(index < 0)
    {
        index += kPlatformCount;
    }
    return index;
}

Vector3 Pendulum::GetPlatformPosition(int platformIndex)
{
    // You'd think this would be simple...just platform->GetPosition right?
    // WRONG! All the platforms have the same position, in the center of the scene.

    // Calculate the current rotation of the platform index we're interested in.
    // The "half platform radians" is needed because we want the rotation of the platform center, not the platform left edge.
    float platformRotation = (kRadiansPerPlatform * 0.5f) + (kRadiansPerPlatform * platformIndex) + mPlatformRotation;

    // Calculate the direction from the center of the map to the platform, based on its current rotation.
    Quaternion rot(Vector3::UnitY, platformRotation);
    Vector3 centerToPlatformDir = rot.Rotate(Vector3::UnitZ);

    // The room center point is almost exactly what we need, but it's a bit too low.
    // Raise it up a bit to align with the top surface of each platform.
    const float kPlatformHalfHeight = 25.0f;
    Vector3 roomCenterPoint = Vector3::Zero + Vector3::UnitY * kPlatformHalfHeight;

    // Estimate the distance of each platform from the center, and use that to get the platform's center point.
    const float kPlatformsRadius = 800.0f;
    Vector3 platformCenterPoint = roomCenterPoint + centerToPlatformDir * kPlatformsRadius;

    // If helpful, some debug drawing can help verify that the math is correct.
    //Debug::DrawLine(roomCenterPoint, platformCenterPoint, Color32::Red);
    //Debug::DrawLine(platformCenterPoint, platformCenterPoint + Vector3::UnitY * 100.0f, Color32::Green);
    return platformCenterPoint;
}

void Pendulum::UpdateGabe(float deltaTime)
{
    if(mGabeState == GabeState::InEntryway)
    {
        // Just set the player to the start position each frame.
        // This is mainly needed to force-reset the position after death.
        mGabeActor->SetPosition(mGabeStartPosition);
    }
    else if(mGabeState == GabeState::Jumping)
    {
        // Keep the player on the platform as it moves, if on any platform.
        if(mGabePlatformIndex >= 0)
        {
            mGabeActor->GetMeshRenderer()->GetOwner()->SetPosition(GetPlatformPosition(mGabePlatformIndex));
        }

        // Interpolate the jump position based on the timer.
        if(mJumpTimer > 0.0f)
        {
            mJumpTimer -= deltaTime;
            mGabeActor->GetMeshRenderer()->GetOwner()->SetPosition(Vector3::Lerp(mJumpInterpFrom, GetPlatformPosition(mJumpToPlatformIndex), 1.0f - mJumpTimer));
        }
    }
    else if(mGabeState == GabeState::OnPlatform)
    {
        // Keep the player on the platform as it moves.
        Vector3 platformPosition = GetPlatformPosition(mGabePlatformIndex);
        mGabeActor->GetMeshRenderer()->GetOwner()->SetPosition(platformPosition);

        // When on a platform, Gabe moves with the platform, but also rotates with it.
        // The correct rotation is always "towards center of room" - we can get that by just negating the platform position (since center of room is origin).
        mGabeActor->SetHeading(Heading::FromDirection(-platformPosition));
    }
    else if(mGabeState == GabeState::DeadOnPlatform)
    {
        // To look correct, after dying on platforms, Gabe's body (and the blood splatter) continue to move with the platforms as they rotate.
        Quaternion rotQuat(Vector3::UnitY, mPlatformRotationSpeed * deltaTime);
        mGabeActor->GetMeshRenderer()->GetOwner()->GetTransform()->RotateAround(Vector3::Zero, rotQuat);
        mGabeMicActor->GetTransform()->RotateAround(Vector3::Zero, rotQuat);
        mGabeBloodActor->GetTransform()->RotateAround(Vector3::Zero, rotQuat);
    }

    // Separately, update interact behavior in each state.
    UpdateGabeInteract();
}

void Pendulum::UpdateGabeInteract()
{
    // Always enable scene interaction by default.
    gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(true);

    // Early out of interaction if scene interact is disabled (due to mouse lock, action playing, etc).
    if(!gSceneManager.GetScene()->GetCamera()->IsSceneInteractAllowed())
    {
        return;
    }

    // Handle jumping to platforms when in the entryway or on a platform.
    if(mGabeState == GabeState::InEntryway || mGabeState == GabeState::OnPlatform)
    {
        // See if the object being hovered by the player's cursor is a platform that we can jump to.
        int hoveredPlatformIndex = -1;
        bool isNextPlatform = false;
        GKObject* hoveredObject = gSceneManager.GetScene()->GetCamera()->RaycastIntoScene(false);
        if(hoveredObject != nullptr)
        {
            // We're hovering something, but check if it's a platform.
            for(int i = 0; i < kPlatformCount; ++i)
            {
                if(hoveredObject == mPlatforms[i])
                {
                    hoveredPlatformIndex = i;
                    break;
                }
            }

            // If it is a platform, see if this platform should be interactive right now.
            // When at the start point, all are interactive. After that, only ones next to you are interactive.
            if(hoveredPlatformIndex >= 0 && mGabeState == GabeState::OnPlatform)
            {
                // Figure out prev and next platform indexes, with wraparound.
                int prevPlatform = mGabePlatformIndex - 1;
                if(prevPlatform < 0) { prevPlatform = kPlatformCount - 1; }
                int nextPlatform = (mGabePlatformIndex + 1) % kPlatformCount;

                // If it isn't next/prev, clear the index variable so we can't interact with it.
                if(hoveredPlatformIndex != prevPlatform && hoveredPlatformIndex != nextPlatform)
                {
                    hoveredPlatformIndex = -1;
                }
                else
                {
                    isNextPlatform = hoveredPlatformIndex == nextPlatform;
                }
            }
        }

        // If the hovered platform index is set, then we CAN interact with the platform!
        if(hoveredPlatformIndex >= 0)
        {
            // Show interact cursor. This one always uses the RED version.
            gCursorManager.UseRedHighlightCursor(1);

            // On click release, we commit to interacting with this platform.
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                if(mGabeState == GabeState::InEntryway)
                {
                    OnForwardJumpStarted();
                }
                else if(isNextPlatform)
                {
                    OnLeftJumpStarted(hoveredPlatformIndex);
                }
                else
                {
                    OnRightJumpStarted(hoveredPlatformIndex);
                }
            }
        }
    }

    // Handle jumping to the pendulum when on the platforms.
    if(mGabeState == GabeState::OnPlatform && mPendulumState != PendulumState::Swinging)
    {
        // Are we hovering the pendulum itself? We may be able to interact with it.
        GKObject* hoveredObject = gSceneManager.GetScene()->GetCamera()->RaycastIntoScene(false);
        if(hoveredObject != nullptr && hoveredObject == mPendulumActor)
        {
            // If hovering the pendulum while it's in the danger zone, see if we're on a platform that we can "reach" it from.
            int platformAtSlot = mPendulumState == PendulumState::InDangerZoneLeft ?
                GetIndexOfPlatformAtLeftPendulumSlot() :
                GetIndexOfPlatformAtRightPendulumSlot();

            // Let's say if you're within two platforms, you can grab the pendulum.
            int prevSlot = platformAtSlot - 1;
            int prevPrevSlot = platformAtSlot - 2;
            if(prevSlot < 0) { prevSlot += kPlatformCount; }
            if(prevPrevSlot < 0) { prevPrevSlot += kPlatformCount; }
            if(mGabePlatformIndex == prevSlot || mGabePlatformIndex == prevPrevSlot)
            {
                // If you can grab it, use the grab cursor.
                // Also disable normal scene interaction, so that clicking doesn't show the verb bar for the pendulum in this case.
                gCursorManager.UseCustomCursor(mGrabCursor, 10);
                gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(false);

                // If clicked, you grab the pendulum!
                if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
                {
                    OnGrabPendulum();
                }
            }
        }
    }

    // Handle jumping to the altar from the pendulum.
    if(mGabeState == GabeState::OnPendulum)
    {
        // In this mode, normal scene interaction isn't allowed.
        gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(false);

        // Check if the altar is hovered by the cursor.
        bool useGrabCursor = false;
        bool useHighlightCursor = false;
        GKObject* hoveredObject = gSceneManager.GetScene()->GetCamera()->RaycastIntoScene(false);
        if(hoveredObject != nullptr && hoveredObject == mAltarActor)
        {
            // If so, check if the pendulum is within the "allowed drop" angle.
            // This just ensures Gabe can't jump off the pendulum when offscreen.
            float pendulumAngle = GetPendulumAngle();
            if(Math::Abs(pendulumAngle) < kAllowedDropAngle)
            {
                // Ok, we can jump off right now, so change the cursor!
                useGrabCursor = true;

                // Clicking the mouse commits to jumping off.
                if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
                {
                    // Show a short animation of Gabe jumping off the pendulum.
                    AnimParams jumpOffPendulumParams;
                    jumpOffPendulumParams.animation = gAssetManager.LoadAnimation("GABJMPOFFPEN", AssetScope::Scene);
                    jumpOffPendulumParams.noParenting = true;
                    gSceneManager.GetScene()->GetAnimator()->Start(jumpOffPendulumParams, [this](){

                        // At the end of Gabe's jumping off anim, it should be pretty clear visually whether he would or wouldn't land on the platform.
                        // We try to match that "look" with an angle range that should allow a safe fall.
                        float pendulumAngle = GetPendulumAngle();
                        if(Math::Abs(pendulumAngle) < kSafeDropAngle)
                        {
                            OnFallToAltar();
                        }
                        else
                        {
                            OnFallToDeath();
                        }
                    });
                }
            }
        }
        else if(hoveredObject != nullptr && hoveredObject->CanInteract())
        {
            useHighlightCursor = true;
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                gActionManager.ExecuteDialogueAction("1SERM44V51");
            }
        }

        // Update cursor.
        if(useGrabCursor)
        {
            gCursorManager.UseCustomCursor(mGrabCursor, 10);
        }
        else if(useHighlightCursor)
        {
            gCursorManager.UseRedHighlightCursor(1);
        }
        else
        {
            gCursorManager.UseDefaultCursor();
        }
    }
}

void Pendulum::ResetAtEntryway()
{
    // Call to Sheepscript to let them know we died and are resetting.
    // The Sheep code shows the death overlay and resets various anims.
    gActionManager.ExecuteSheepAction("wait CallSheep(\"TE3\", \"Die$\")");

    // Back to entryway.
    mGabeState = GabeState::InEntryway;
    gGameProgress.ClearFlag("Te3GabeAtAltar"); // do this AFTER Sheep call, since Sheep code uses this during reset logic

    // Reset platform index.
    mGabePlatformIndex = -1;

    // Reset jump tracking variables.
    mJumpToPlatformIndex = -1;
    mJumpTimer = 0.0f;

    // Go back to idling.
    gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);

    // Make sure forced cinematics are disabled, in case Gabe died falling from pedulum.
    gSceneManager.GetScene()->GetCamera()->SetForcedCinematicMode(false);

    // Make sure door is opened again.
    Animation* doorCloseAnim = gAssetManager.LoadAnimation("TE3_DOORCLOSE", AssetScope::Scene);
    gSceneManager.GetScene()->GetAnimator()->Sample(doorCloseAnim, 0);
}

void Pendulum::OnForwardJumpStarted()
{
    mGabeState = GabeState::Jumping;

    // The original game doesn't consider this an action - you can save during the forward jump animation.
    // However, this is much harder to support for the way we're doing save games. So we'll disallow saving during this jump.
    gActionManager.StartManualAction();

    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpForwardStartAnim;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

        // Cut to a specific camera (this hides Gabe obviously snapping to a specific tile at the apex of his jump).
        gSceneManager.GetScene()->SetCameraPosition("JUMP_OUT");

        // Position player at the platform that is in front of the entryway when the forward jump happens.
        int platformIndex = GetIndexOfPlatformAtEntryway();
        mGabeActor->SetPosition(GetPlatformPosition(platformIndex));
        mGabePlatformIndex = platformIndex;

        // Play animation of entry door closing.
        Animation* doorCloseAnim = gAssetManager.LoadAnimation("TE3_DOORCLOSE", AssetScope::Scene);
        gSceneManager.GetScene()->GetAnimator()->Start(doorCloseAnim);

        // Play jump end animation.
        mJumpAnimParams.animation = mJumpForwardEndAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

            // We are now on the platform.
            mGabeState = GabeState::OnPlatform;

            // No longer in an action.
            gActionManager.FinishManualAction();

            // Play idle anim again.
            gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
        });
    });
}

void Pendulum::OnLeftJumpStarted(int toPlatformIndex)
{
    mGabeState = GabeState::Jumping;

    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // This is kind of a HACK to ensure the jump anim looks as smooth as possible.
    // I noticed Gabe's 3D model would glitch for one frame to the wrong location. Probably some complexity of the interplay between the GKActor and its Model Actor...
    // A quick fix is to just force both to the correct position at this time.
    mGabeActor->SetPosition(GetPlatformPosition(mGabePlatformIndex));
    mGabeActor->GetMeshRenderer()->GetOwner()->SetPosition(GetPlatformPosition(mGabePlatformIndex));

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpLeftStartAnim;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

        mJumpInterpFrom = mGabeActor->GetMeshRenderer()->GetOwner()->GetPosition();
        mJumpToPlatformIndex = toPlatformIndex;
        mJumpTimer = 1.0f;

        mJumpAnimParams.animation = mJumpLeftMiddleAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

            mGabeActor->SetPosition(GetPlatformPosition(toPlatformIndex));
            mGabePlatformIndex = toPlatformIndex;

            // Play jump end animation.
            mJumpAnimParams.animation = mJumpLeftEndAnim;
            gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

                // We're on the platform, so start to rotate the player with the platform.
                mGabeState = GabeState::OnPlatform;

                // Play idle anim again.
                gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
            });
        });
    });
}

void Pendulum::OnRightJumpStarted(int toPlatformIndex)
{
    mGabeState = GabeState::Jumping;

    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // This is kind of a HACK to ensure the jump anim looks as smooth as possible.
    // I noticed Gabe's 3D model would glitch for one frame to the wrong location. Probably some complexity of the interplay between the GKActor and its Model Actor...
    // A quick fix is to just force both to the correct position at this time.
    mGabeActor->SetPosition(GetPlatformPosition(mGabePlatformIndex));
    mGabeActor->GetMeshRenderer()->GetOwner()->SetPosition(GetPlatformPosition(mGabePlatformIndex));

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpRightStartAnim;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

        mJumpInterpFrom = mGabeActor->GetMeshRenderer()->GetOwner()->GetPosition();
        mJumpToPlatformIndex = toPlatformIndex;
        mJumpTimer = 1.0f;

        mJumpAnimParams.animation = mJumpRightMiddleAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

            mGabeActor->SetPosition(GetPlatformPosition(toPlatformIndex));
            mGabePlatformIndex = toPlatformIndex;

            // Play jump end animation.
            mJumpAnimParams.animation = mJumpRightEndAnim;
            gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

                // We're on the platform, so start to rotate the player with the platform.
                mGabeState = GabeState::OnPlatform;

                // Play idle anim again.
                gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
            });
        });
    });
}

void Pendulum::OnPendulumPlatformDeath(bool onLeftSide, bool pendulumMovingLeft)
{
    mGabeState = GabeState::Dying;

    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // Cut to zoomed in death camera.
    // No idea why these are named this way, but the left side is "HIGH" and right side is "LOW".
    gSceneManager.GetScene()->SetCameraPosition(onLeftSide ? "KILL_HIGH" : "KILL_LOW");

    // Depending on whether this death is at the left or right pendulum slot, and whether the pendulum is coming or going, play the appropriate animation.
    std::string deathAnim;
    if(onLeftSide)
    {
        deathAnim = pendulumMovingLeft ? "GABTE3PNFTH" : "GABTE3PNBKH";
    }
    else
    {
        deathAnim = pendulumMovingLeft ? "GABTE3PNBKL" : "GABTE3PNFTL";
    }

    // Play a gruesome death sequence.
    std::string actionStr = StringUtil::Format("StartDialogue(\"1REGJ67Q81\", 1); wait StartAnimation(\"%s\")", deathAnim.c_str());
    gActionManager.ExecuteSheepAction(actionStr, [this, onLeftSide](const Action* action){

        // After falling/dying, the player should move with the platforms again.
        mGabeState = GabeState::DeadOnPlatform;

        // Cut to high camera and wait a beat, so you can feel bad.
        gSceneManager.GetScene()->SetCameraPosition(onLeftSide ? "AFTERKILL_HIGH" : "AFTERKILL_LOW");
        gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

            // Reset ego at entryway, so the player can try again if they choose "retry" on the death screen.
            ResetAtEntryway();
        });
    });
}

void Pendulum::OnGrabPendulum()
{
    mGabeState = GabeState::OnPendulum;

    // Pretend this is an action so we don't have to support saving during this tiny cutscene.
    gActionManager.StartManualAction();

    // Do a camera cut to hide the model swap shenanigans we're attempting here.
    gSceneManager.GetScene()->SetCameraPosition(mPendulumState == PendulumState::InDangerZoneLeft ? "KILL_HIGH" : "KILL_LOW");

    // From this point on, the camera is locked, and the player can't move it around.
    gSceneManager.GetScene()->GetCamera()->SetForcedCinematicMode(true);

    // Swap to version of pendulum with Gabe holding on.
    UseGabePendulum();

    // If Gabe grabs the pendulum on the right side, he grabs it from the back instead of the front.
    // An easy way to make this work is to simply flip the z-scale of the pendulum actor in that case.
    if(mPendulumState == PendulumState::InDangerZoneRight)
    {
        mGabePendulumActor->SetScale(Vector3(1.0f, 1.0f, -1.0f));
    }
    else
    {
        mGabePendulumActor->SetScale(Vector3::One);
    }

    // Play the animation of Gabe jumping up onto the pendulum.
    AnimParams grabPendulumParams;
    grabPendulumParams.animation = gAssetManager.LoadAnimation("GABJMPPNDULM", AssetScope::Scene);
    grabPendulumParams.noParenting = true;
    gSceneManager.GetScene()->GetAnimator()->Start(grabPendulumParams, [](){

        // After the anim, change to a set camera position of the altar.
        // From here, the player must try to drop off the pendulum and land on the altar.
        gSceneManager.GetScene()->SetCameraPosition("LONG_ALTAR");

        // Done with little cutscene.
        gActionManager.FinishManualAction();
    });

    // You get some points for this action!
    // Note that, if you die and retry after this point, you still have these points. This is how the original game functioned too.
    gGameProgress.ChangeScore("e_temple_grab_pendulum");
}

void Pendulum::OnFallToDeath()
{
    // Enter the dying state.
    mGabeState = GabeState::Dying;

    // The original game doesn't consider this an "action" - but it's better if we don't let people save during this part.
    gActionManager.StartManualAction();

    // Toggle back to non-Gabe version of pendulum.
    UseNormalPendulum();

    // Position Gabe right under the pendulum.
    // This is janky, but the camera cut masks the position change.
    // The camera cut also purposely makes it difficult to gauge depth, but I think the cutscene ultimately looks better if you move Gabe closer to the ground.
    Vector3 pendulumPos = mPendulumActor->GetTransform()->GetPosition();
    pendulumPos.y -= 250.0f;
    mGabeActor->SetPosition(pendulumPos);

    // Switch to a different camera angle.
    gSceneManager.GetScene()->SetCameraPosition("FALL_DOWN");

    // Play the fall death anim (RIP).
    Animation* fallDeathAnim = gAssetManager.LoadAnimation("GABEFALLDEATH", AssetScope::Scene);
    gSceneManager.GetScene()->GetAnimator()->Start(fallDeathAnim, [this](){

        // Wait a beat so you can see your mistake.
        Timers::AddTimerSeconds(2.0f, [this](){

            // Done with manual action.
            gActionManager.FinishManualAction();

            // Reset ego at entryway, so the player can try again if they choose "retry" on the death screen.
            ResetAtEntryway();
        });
    });
}

void Pendulum::OnFallToAltar()
{
    // Go back to the normal pendulum.
    UseNormalPendulum();

    // Force Gabe to the altar's position/heading.
    const ScenePosition* scenePos = gSceneManager.GetScene()->GetPosition("ALTAR_LAND");
    mGabeActor->SetPosition(scenePos->position);
    mGabeActor->SetHeading(scenePos->heading);

    // Switch to a different camera angle, to hide the transition.
    gSceneManager.GetScene()->SetCameraPosition("ALTAR_UP");

    // From here, the player is allowed to move the camera around again.
    gSceneManager.GetScene()->GetCamera()->SetForcedCinematicMode(false);

    // We're now on the altar.
    mGabeState = GabeState::AtAltar;
    gGameProgress.SetFlag("Te3GabeAtAltar");

    // Force scene interactive again.
    gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(true);

    // Let the Sheepscript for this room handle the rest.
    gActionManager.ExecuteSheepAction("wait CallSheep(\"TE3\", \"GabeOnPillar$\")");
}
