//
// Clark Kromenaker
//
// The deadly pendulum trap swinging across one of the rooms in the final stretch of the game.
//
// This component encompasses the pendulum swing behavior, but it also manages the rotating of the platforms around the pendulum,
// as well as the logic related to moving between platforms, jumping to the pendulum, getting killed by the pendulum, etc.
//
#pragma once
#include "Component.h"

#include "Animator.h"
#include "GMath.h"
#include "Vector3.h"

class Animation;
class GKObject;

class Pendulum : public Component
{
public:
    Pendulum(Actor* owner);

protected:
    void OnUpdate(float deltaTime) override;

private:
    //
    // PENDULUM SWINGING
    // 
    // The approximate radius of the pendulum arm.
    static constexpr float kArmRadius = 2000.0f;

    // The point around which the pendulum is pivoting.
    Vector3 mPivotPoint;

    // The max angle of rotation (the minimum is the negative of this).
    float mMaxAngle = 0.0f;

    // How long it takes for a pendulum to complete one cycle (swing from min to max angle and back again).
    float mCycleDuration = 0.0f;

    // Times how far the pendulum is into its current cycle.
    float mCycleTimer = 0.0f;

    void UpdatePendulum(float deltaTime);
    void SetAngle(float angle);

    //
    // PENDULUM PLATFORMS
    // 
    // The platforms rotating around the pendulum.
    static const int kPlatformCount = 24;
    GKObject* mPlatforms[kPlatformCount] = { 0 };

    // The platforms are arranged in a circle, which each platform making up a portion of the circle's boundary.
    // Based on this, we can calculate how many radians of the 2pi total are used by each platform.
    static constexpr float kRadiansPerPlatform = Math::k2Pi / kPlatformCount;

    // Platforms rotate around the room, using the same angle value for all.
    // Stored here for convenience, to avoid converting to/from quaternion when we need it.
    float mPlatformRotation = 0.0f;

    void UpdatePlatforms(float deltaTime);

    int GetIndexOfPlatformAtEntryway();
    int GetIndexOfPlatformAtLeftPendulumSlot();
    int GetIndexOfPlatformAtRightPendulumSlot();
    Vector3 GetPlatformPosition(int index);

    //
    // PLAYER LOGIC
    //
    GKObject* mPlayer = nullptr;
    GKObject* mPlayerBlood = nullptr;

    // Cached start position for the player.
    Vector3 mPlayerStartPosition;
    
    // The index of the platform the player is currently on.
    int mPlayerPlatformIndex = -1;

    // If true, we set the player's rotation to match the platform they're on.
    bool mSetPlayerRotationOnPlatform = false;

    // Set to true when Gabe dies.
    bool mPlayerDead = false;

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

    void UpdatePlayer(float deltaTime);
    
    void OnForwardJumpStarted();
    void OnLeftJumpStarted(int toPlatformIndex);
    void OnRightJumpStarted(int toPlatformIndex);
};