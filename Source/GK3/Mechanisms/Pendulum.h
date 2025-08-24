//
// Clark Kromenaker
//
// The deadly pendulum trap swinging across one of the rooms in the final stretch of the game.
//
// This component encompasses the pendulum swing behavior, but it also manages the rotating of the platforms around the pendulum,
// as well as the logic related to moving between platforms, jumping to the pendulum, getting killed by the pendulum, etc.
//
#pragma once
#include "Actor.h"

#include "Animator.h"
#include "GMath.h"
#include "Vector3.h"

class Animation;
class Cursor;
class GKObject;
class PersistState;

class Pendulum : public Actor
{
public:
    Pendulum();

    void OnPersist(PersistState& ps);

protected:
    void OnUpdate(float deltaTime) override;

private:
    //
    // PENDULUM SWINGING
    //
    // The approximate radius of the pendulum arm.
    static constexpr float kPendulumArmRadius = 2000.0f;

    // The scene contains two pendulums - one without Gabe, one with Gabe.
    // Hold references to both so we can easily swap between the two.
    GKObject* mNormalPendulumActor = nullptr;
    GKObject* mGabePendulumActor = nullptr;

    // The object being used as the pendulum. This changes when Gabe is riding it.
    Actor* mPendulumActor = nullptr;

    // The point around which the pendulum is pivoting.
    Vector3 mPendulumPivotPoint;

    // The max angle of rotation (the minimum is the negative of this).
    float mPendulumMaxAngle = 0.0f;

    // How long it takes for a pendulum to complete one cycle (swing from min to max angle and back again).
    float mPendulumCycleDuration = 0.0f;

    // Times how far the pendulum is into its current cycle.
    float mPendulumCycleTimer = 0.0f;

    // Tracks whether the pendulum is in the "danger zones" on the left/right side of the swing arc.
    // When in these zones, the pendulum CAN kill Gabe, but it's also when you can grab the pendulum and ride it.
    enum class PendulumState
    {
        Swinging,
        InDangerZoneLeft,
        InDangerZoneRight
    };
    PendulumState mPendulumState = PendulumState::Swinging;

    void UpdatePendulum(float deltaTime);

    void SetPendulumActorRotation(float angle);
    float GetPendulumAngle();

    void UseNormalPendulum();
    void UseGabePendulum();

    //
    // PENDULUM PLATFORMS
    //
    // The platforms rotating around the pendulum.
    static const int kPlatformCount = 24;
    GKObject* mPlatforms[kPlatformCount] = { 0 };

    // The platforms are arranged in a circle, which each platform making up a portion of the circle's boundary.
    // Based on this, we can calculate how many radians of the 2pi total are used by each platform.
    static constexpr float kRadiansPerPlatform = Math::k2Pi / kPlatformCount;

    // Speed of platform rotation, in radians per second.
    float mPlatformRotationSpeed = 0.0f;

    // Platforms rotate around the room, using the same angle value for all.
    // Stored here for convenience, to avoid converting to/from quaternion when we need it.
    float mPlatformRotation = 0.0f;

    void UpdatePlatforms(float deltaTime);

    int GetIndexOfPlatformAtEntryway();
    int GetIndexOfPlatformAtLeftPendulumSlot();
    int GetIndexOfPlatformAtRightPendulumSlot();
    Vector3 GetPlatformPosition(int platformIndex);

    //
    // PLAYER LOGIC
    //
    // Gabe himself.
    GKObject* mGabeActor = nullptr;

    // The Mic is attached to Gabe's head, and the blood is used in some death sequences.
    GKObject* mGabeMicActor = nullptr;
    GKObject* mGabeBloodActor = nullptr;

    // The altar is used as a click target when dropping off the pendulum.
    GKObject* mAltarActor = nullptr;

    // Cached start position for Gabe.
    Vector3 mGabeStartPosition;

    // The index of the platform Gabe is currently on.
    int mGabePlatformIndex = -1;

    // This room's puzzle is fairly complex, and we need to manage Gabe's state in code.
    enum class GabeState
    {
        InEntryway,
        Jumping,
        OnPlatform,
        OnPendulum,
        AtAltar,
        Done,
        Dying,
        DeadOnPlatform,
    };
    GabeState mGabeState = GabeState::InEntryway;

    // Cached grab cursor.
    Cursor* mGrabCursor = nullptr;

    // Animations used for player idle and jumping in different directions.
    Animation* mIdleAnim = nullptr;

    Animation* mJumpForwardStartAnim = nullptr;
    Animation* mJumpForwardEndAnim = nullptr;

    Animation* mJumpLeftStartAnim = nullptr;
    Animation* mJumpLeftMiddleAnim = nullptr;
    Animation* mJumpLeftEndAnim = nullptr;

    Animation* mJumpRightStartAnim = nullptr;
    Animation* mJumpRightMiddleAnim = nullptr;
    Animation* mJumpRightEndAnim = nullptr;

    // Anim params used for idle and jumping anim playback.
    AnimParams mIdleAnimParams;
    AnimParams mJumpAnimParams;

    // When Gabe jumps from platform to platform, an interp is used to simulate the jump movement.
    Vector3 mJumpInterpFrom;
    int mJumpToPlatformIndex = -1;
    float mJumpTimer = 0.0f;

    // When dropping to the altar, some parameters for when dropping is allowed, and what's considered a "safe" drop.
    static constexpr float kAllowedDropAngle = Math::ToRadians(15.0f);
    static constexpr float kSafeDropAngle = Math::ToRadians(3.0f);

    void UpdateGabe(float deltaTime);
    void UpdateGabeInteract();

    void ResetAtEntryway();

    void OnForwardJumpStarted();
    void OnLeftJumpStarted(int toPlatformIndex);
    void OnRightJumpStarted(int toPlatformIndex);

    void OnPendulumPlatformDeath(bool onLeftSide, bool pendulumMovingLeft);

    void OnGrabPendulum();

    void OnFallToDeath();
    void OnFallToAltar();
};