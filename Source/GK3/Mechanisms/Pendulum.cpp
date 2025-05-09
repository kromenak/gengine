#include "Pendulum.h"

#include "Actor.h"
#include "Debug.h"
#include "Interpolate.h"

#include "ActionManager.h"
#include "Animator.h"
#include "AssetManager.h"
#include "Camera.h"
#include "CursorManager.h"
#include "GameCamera.h"
#include "GKActor.h"
#include "InputManager.h"
#include "MeshRenderer.h"
#include "Ray.h"
#include "SceneManager.h"
#include "UICanvas.h"

Pendulum::Pendulum(Actor* owner) : Component(owner)
{
    //TODO: Read these values from PENDULUM.TXT
    float platformRevolutionDuration = 50.0f;
    int platformsPerPendulumCycle = 6;
    mMaxAngle = Math::ToRadians(24.5f);

    // The amount of time it takes for the pendulum to complete one cycle is derived in a truly deranged way.
    // We know how many platforms should pass the pendulum between cycles.
    // We know the speeds of the platforms. So...let's figure this out.

    // The platforms rotate in a circle over the course of 50 seconds. So, the radians of movement per second are 2pi/50.
    float platformRadiansPerSecond = Math::k2Pi / platformRevolutionDuration;

    // The amount of time it takes for a single platform to pass is then the radians per platform divided by radians per second.
    float platformDuration = kRadiansPerPlatform / platformRadiansPerSecond;

    // The pendulum is supposed to complete a cycle every 6 pendulum cycles, so we can now calculate how much time that should be.
    mCycleDuration = platformDuration * platformsPerPendulumCycle;

    // Calculate pivot point based on initial actor position.
    // We're assuming the pendulum is initially at the bottom of the oscillation arc. So pivot point is straight up.
    mPivotPoint = GetOwner()->GetPosition() + Vector3::UnitY * kArmRadius;

    // Set pendulum initially to its max angle.
    SetAngle(mMaxAngle);

    // Get pointers to all platforms.
    std::string platformModelName = "te3_r01";
    for(int i = 1; i <= kPlatformCount; ++i)
    {
        platformModelName[5] = '0' + (i / 10);
        platformModelName[6] = '0' + (i % 10);
        mPlatforms[i - 1] = gSceneManager.GetScene()->GetSceneObjectByModelName(platformModelName);
    }

    // Get player start position.
    mPlayerStartPosition = gSceneManager.GetScene()->GetPosition("LookOut")->position;

    // Get player, too.
    mPlayer = gSceneManager.GetScene()->GetEgo();
    mPlayerBlood = gSceneManager.GetScene()->GetSceneObjectByModelName("groundblood");

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

    // Because this Pendulum sequence is so complex, all animations are controlled in this class, as opposed to via the fidget or walk systems.
    // Start by looping Gabe's idle animation.
    mIdleAnimParams.animation = mIdleAnim;
    mIdleAnimParams.loop = true;
    gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
}

void Pendulum::OnUpdate(float deltaTime)
{
    UpdatePendulum(deltaTime);
    UpdatePlatforms(deltaTime);
    UpdatePlayer(deltaTime);
}

void Pendulum::UpdatePendulum(float deltaTime)
{
    // Increase current cycle timer.
    mCycleTimer += deltaTime;

    // Use cubic interpolation to calculate the pendulum's angle between min and max along the desired duration.
    // The duration represents a full swing out AND back, so we need to account for switching direction halfway through the duration.
    float angle = 0.0f;
    float halfCycleDuration = mCycleDuration * 0.5f;
    int moveDir = 0;
    if(mCycleTimer <= halfCycleDuration)
    {
        angle = Interpolate::CubicInOut(mMaxAngle, -mMaxAngle, mCycleTimer / halfCycleDuration);
        moveDir = -1;
    }
    else
    {
        angle = Interpolate::CubicInOut(-mMaxAngle, mMaxAngle, (mCycleTimer - halfCycleDuration) / halfCycleDuration);
        moveDir = 1;
    }

    // Update the angle used by the in-game 3D object.
    SetAngle(angle);

    // If we exceed the duration, subtract it to get back within a valid range.
    if(mCycleTimer >= mCycleDuration)
    {
        mCycleTimer -= mCycleDuration;
    }

    // If the pendulum is near the wall slots on either the left or right sides of the room, we need to check if Gabe is going to die.
    float maxAngleDiff = Math::Abs(mMaxAngle - Math::Abs(angle));
    if(maxAngleDiff < Math::ToRadians(5.0f))
    {
        bool onLeftSide = angle > 0.0f;
        if(onLeftSide && GetIndexOfPlatformAtLeftPendulumSlot() == mPlayerPlatformIndex)
        {
            gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);
            mPlayerPlatformIndex = -2;
            mSetPlayerRotationOnPlatform = false;
            mPlayerDead = true;
            
            // Cut to low death camera.
            gSceneManager.GetScene()->SetCameraPosition("KILL_HIGH");

            // Play a gruesome death sequence.
            gActionManager.ExecuteSheepAction("StartDialogue(\"1REGJ67Q81\", 1); wait StartAnimation(\"GABTE3PNBKH\")", [this](const Action* action){
                mSetPlayerRotationOnPlatform = true;

                // Cut to high camera and wait a beat, so you can feel bad.
                gSceneManager.GetScene()->SetCameraPosition("AFTERKILL_HIGH");
                Timers::AddTimerSeconds(3.0f, [this](){

                    // Call to Sheep to handle the rest.
                    gActionManager.ExecuteSheepAction("wait CallSheep(\"TE3\", \"Die$\")");

                    mSetPlayerRotationOnPlatform = false;
                    mPlayerDead = false;
                    mPlayerPlatformIndex = -1;
                });
            });
        }

        //printf("Danger Zone!\n");
    }
}

void Pendulum::SetAngle(float angle)
{
    // We're assuming the pendulum rotates about the z-axis.
    Quaternion rot(Vector3::UnitZ, angle);

    // Figure out which direction corresponds to this angle, assuming straight down is angle of 0.0f.
    Vector3 rotated = rot.Rotate(-Vector3::UnitY);
    //Debug::DrawLine(mPivotPoint, mPivotPoint + rotated * kArmRadius, Color32::Red, 60.0f);

    // Rotate the pendulum to that rotation.
    GetOwner()->GetTransform()->SetRotation(rot);

    // For whatever reason, the pendulum model doesn't have its pivot at the pivot point.
    // To compensate for this, we also need to position the model at a corrected position, calculated based on the arm rotation and length.
    GetOwner()->GetTransform()->SetPosition(mPivotPoint + rotated * kArmRadius);
}

void Pendulum::UpdatePlatforms(float deltaTime)
{
    // Update rotation.
    const float kRotationSpeed = Math::k2Pi / 50.0f;
    mPlatformRotation += kRotationSpeed * deltaTime;

    // Apply rotation to all platforms.
    Heading heading = Heading::FromRadians(mPlatformRotation);
    for(GKObject* platform : mPlatforms)
    {
        platform->SetHeading(heading);
    }

    for(int i = 0; i < kPlatformCount; ++i)
    {
        GetPlatformPosition(i);
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
    int platformsPastEntry = rotation / kRadiansPerPlatform;

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

Vector3 Pendulum::GetPlatformPosition(int index)
{
    // You'd think this would be simple...just platform->GetPosition right?
    // WRONG! All the platforms have the same position, in the center of the map.

    Vector3 dir = Vector3::UnitZ;
    Quaternion rot(Vector3::UnitY, Math::ToRadians(7.5f + (15.0f * index)) + mPlatformRotation);
    dir = rot.Rotate(dir);

    const float kPlatformsRadius = 800.0f;
    const float kPlatformHalfHeight = 25.0f;

    Vector3 roomCenterPoint = Vector3::Zero + Vector3::UnitY * kPlatformHalfHeight;
    Vector3 platformCenterPoint = roomCenterPoint + dir * kPlatformsRadius;
    Debug::DrawLine(roomCenterPoint, platformCenterPoint, Color32::Red);
    Debug::DrawLine(platformCenterPoint, platformCenterPoint + Vector3::UnitY * 100.0f, Color32::Green);
    return platformCenterPoint;
}

void Pendulum::UpdatePlayer(float deltaTime)
{
    // Set player position, either to start position or to the current platform position.
    if(mPlayerPlatformIndex == -1)
    {
        mPlayer->SetPosition(mPlayerStartPosition);
    }
    else if(mPlayerPlatformIndex >= 0)
    {
        Vector3 platformPosition = GetPlatformPosition(mPlayerPlatformIndex);
        mPlayer->GetMeshRenderer()->GetOwner()->SetPosition(platformPosition);

        // When on a platform, Gabe moves with the platform, but also rotates with it (usually).
        // The correct rotation is always "towards center of room" - we can get that by just negating the platform position (since center of room is origin).
        if(mSetPlayerRotationOnPlatform)
        {
            platformPosition.y = 0.0f;
            mPlayer->SetHeading(Heading::FromDirection(-platformPosition));
        }
    }

    if(mSetPlayerRotationOnPlatform && mPlayerDead)
    {
        const float kRotationSpeed = Math::k2Pi / 50.0f;
        Quaternion rotQuat(Vector3::UnitY, kRotationSpeed * deltaTime);
        mPlayer->GetMeshRenderer()->GetOwner()->GetTransform()->RotateAround(Vector3::Zero, rotQuat);
        mPlayerBlood->GetTransform()->RotateAround(Vector3::Zero, rotQuat);
    }

    // Don't allow interacting under certain conditions.
    if(gActionManager.IsActionPlaying() || gInputManager.IsMouseLocked() || UICanvas::DidWidgetEatInput() || mPlayerDead)
    {
        return;
    }

    // Get a ray into the world from the game camera and mouse position.
    GKObject* hoveredObject = gSceneManager.GetScene()->GetCamera()->RaycastIntoScene(false);

    // Do a raycast to see if the pointer is over a platform that can be jumped to.
    int hoveredPlatformIndex = -1;
    bool isNextPlatform = false;
    if(hoveredObject != nullptr)
    {
        // Figure out which platform it is.
        for(int i = 0; i < kPlatformCount; ++i)
        {
            if(hoveredObject == mPlatforms[i])
            {
                hoveredPlatformIndex = i;
                break;
            }
        }
        
        // See if this platform should be interactive right now.
        // When at the start point, all are interactive. After that, only ones next to you are interactive.
        if(hoveredPlatformIndex >= 0 && mPlayerPlatformIndex >= 0)
        {
            // Figure out prev and next platform indexes, with wraparound.
            int prevPlatform = mPlayerPlatformIndex - 1;
            if(prevPlatform < 0) { prevPlatform = kPlatformCount - 1; }
            int nextPlatform = (mPlayerPlatformIndex + 1) % kPlatformCount;

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

    // If the hovered platform is still set, then we CAN interact with it!
    if(hoveredPlatformIndex >= 0)
    {
        // Show interact cursor. This one always uses the RED version.
        gCursorManager.UseRedHighlightCursor();

        // On click, we commit to interacting with this platform.
        if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
        {
            if(mPlayerPlatformIndex == -1)
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

    // During a left/right jump, we use an interp to move Gabe through the air mid-jump.
    if(mJumpTimer > 0.0f)
    {
        mJumpTimer -= deltaTime;
        mPlayer->GetMeshRenderer()->GetOwner()->SetPosition(Vector3::Lerp(mJumpInterpFrom, GetPlatformPosition(mJumpToPlatformIndex), 1.0f - mJumpTimer));
    }
}

void Pendulum::OnForwardJumpStarted()
{
    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // The player isn't yet "on" the platform, so don't rotate the player with the platforms yet.
    mSetPlayerRotationOnPlatform = false;

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpForwardStartAnim;
    mJumpAnimParams.allowMove = true;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

        // Cut to a specific camera (this hides Gabe obviously snapping to a specific tile at the apex of his jump).
        gSceneManager.GetScene()->SetCameraPosition("JUMP_OUT");

        // Position player at the platform that is in front of the entryway when the forward jump happens.
        int platformIndex = GetIndexOfPlatformAtEntryway();
        mPlayer->SetPosition(GetPlatformPosition(platformIndex));
        mPlayerPlatformIndex = platformIndex;

        // Play jump end animation.
        mJumpAnimParams.animation = mJumpForwardEndAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

            // We're on the platform, so start to rotate the player with the platform.
            mSetPlayerRotationOnPlatform = true;

            // Play idle anim again.
            gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
        });
    });
}

void Pendulum::OnLeftJumpStarted(int toPlatformIndex)
{
    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // The player isn't yet "on" the platform, so don't rotate the player with the platforms yet.
    mSetPlayerRotationOnPlatform = false;

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpLeftStartAnim;
    mJumpAnimParams.allowMove = true;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

        mJumpInterpFrom = mPlayer->GetMeshRenderer()->GetOwner()->GetPosition();
        mJumpToPlatformIndex = toPlatformIndex;
        mJumpTimer = 1.0f;

        mJumpAnimParams.animation = mJumpLeftMiddleAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

            mPlayer->SetPosition(GetPlatformPosition(toPlatformIndex));
            mPlayerPlatformIndex = toPlatformIndex;

            // Play jump end animation.
            mJumpAnimParams.animation = mJumpLeftEndAnim;
            gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

                // We're on the platform, so start to rotate the player with the platform.
                mSetPlayerRotationOnPlatform = true;

                // Play idle anim again.
                gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
            });
        });
    });
}

void Pendulum::OnRightJumpStarted(int toPlatformIndex)
{
    // Stop idle anim.
    gSceneManager.GetScene()->GetAnimator()->Stop(mIdleAnim);

    // The player isn't yet "on" the platform, so don't rotate the player with the platforms yet.
    mSetPlayerRotationOnPlatform = false;

    // Play the start jump anim.
    mJumpAnimParams.animation = mJumpRightStartAnim;
    mJumpAnimParams.allowMove = true;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

        mJumpInterpFrom = mPlayer->GetMeshRenderer()->GetOwner()->GetPosition();
        mJumpToPlatformIndex = toPlatformIndex;
        mJumpTimer = 1.0f;

        mJumpAnimParams.animation = mJumpRightMiddleAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, toPlatformIndex](){

            mPlayer->SetPosition(GetPlatformPosition(toPlatformIndex));
            mPlayerPlatformIndex = toPlatformIndex;

            // Play jump end animation.
            mJumpAnimParams.animation = mJumpRightEndAnim;
            gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

                // We're on the platform, so start to rotate the player with the platform.
                mSetPlayerRotationOnPlatform = true;

                // Play idle anim again.
                gSceneManager.GetScene()->GetAnimator()->Start(mIdleAnimParams);
            });
        });
    });
}
