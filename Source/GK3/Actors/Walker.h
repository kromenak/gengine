//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of moving an actor's legs
// and walking from point to point in the game world.
//
// A Walker Component is always attached to a GKActor Actor. It is safe to make this assumption.
//
#pragma once
#include "Component.h"

#include <functional>
#include <vector>

#include "Vector3.h"

class Animation;
struct CharacterConfig;
class GKActor;
class GKObject;
class GKProp;
class Heading;
class PersistState;
class Texture;
class VertexAnimation;
class WalkerBoundary;

class Walker : public Component
{
    TYPEINFO_SUB(Walker, Component);
public:
    Walker(Actor* owner);
    ~Walker();

    void SetWalkerBoundary(WalkerBoundary* walkerBoundary);
    void SetCharacterConfig(const CharacterConfig& characterConfig);
    void SetWalkAnims(Animation* startAnim, Animation* loopAnim,
                      Animation* startTurnLeftAnim, Animation* startTurnRightAnim);

    void WalkToBestEffort(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToExact(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToGas(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToSee(GKObject* target, const std::function<void()>& finishCallback);

    void WalkOutOfRegion(int regionIndex, const Vector3& exitPosition, const Heading& exitHeading, const std::function<void()>& finishCallback);

    void SkipToEnd(bool alsoSkipWalkEndAnim = false);
    void StopWalk();

    bool AtPosition(const Vector3& position, float maxDistance = kAtNodeDist);
    bool IsWalking() const { return mWalkActions.size() > 0; }
    bool IsWalkingExceptTurn() const { return IsWalking() && mWalkActions.back() != WalkOp::TurnToFace; }
    Vector3 GetDestination() const { return mPath.size() > 0 ? mPath.front() : Vector3::Zero; }

    bool IsWalkAnimation(VertexAnimation* vertexAnim) const;

    void OnPersist(PersistState& ps);

protected:
    void OnEnable() override;
    void OnDisable() override;
    void OnUpdate(float deltaTime) override;

private:
    // When this close to a position/heading, we say you are "at" the position/heading.
    static constexpr float kAtNodeDist = 12.25f;
    static constexpr float kAtNodeDistSq = kAtNodeDist * kAtNodeDist;
    static constexpr float kAtHeadingRadians = Math::ToRadians(4.0f);

    // Turn speeds. When walking, turn faster when the next path node is very close - slower when farther away.
    // A faster speed is used for turning in place when not walking.
    static constexpr float kWalkTurnSpeedMin = Math::kPiOver2;
    static constexpr float kWalkTurnSpeedMax = Math::k2Pi * 2;
    static constexpr float kTurnSpeed = Math::k2Pi;

    // CONFIG
    // Walker's owner, as a GKActor.
    GKActor* mGKOwner = nullptr;

    // Config is vital for walker to function - contains things like walk anims and hip position data.
    const CharacterConfig* mCharConfig = nullptr;

    // Walker boundary currently being used.
    WalkerBoundary* mWalkerBoundary = nullptr;

    // Walk animations currently in use.
    // Need to store separate from CharacterConfig because they can be overridden in some circumstances.
    Animation* mWalkStartAnim = nullptr;
    Animation* mWalkLoopAnim = nullptr;
    Animation* mWalkStartTurnLeftAnim = nullptr;
    Animation* mWalkStartTurnRightAnim = nullptr;

    // WALK PLANNING
    // When a walk begins, a "plan" is generated and stored in the walk ops vector.
    // Ex: we may calculate plan: turn right, then follow path, then end move, then turn to face heading.
    enum class WalkOp
    {
        None,
        FollowPathStart,
        FollowPathStartTurnLeft,
        FollowPathStartTurnRight,
        FollowPath,
        FollowPathEnd,
        TurnToFace
    };
    std::vector<WalkOp> mWalkActions;  // Walk ops are in reverse - next action to perform is back().

    // In at least one case, it's helpful to know what the previous walk operation was.
    WalkOp mPrevWalkOp = WalkOp::None;

    // The amount of time spent on the current walk action.
    float mCurrentWalkActionTimer = 0.0f;

    // The path to follow when in the FollowPath state.
    // Note that this path is BACKWARDS - the goal is front() and next node to go to is back().
    std::vector<Vector3> mPath;

    // The final position to be at when the walk ends.
    // This usually matches mPath.front(), but kept separate so we can use it after the path has been followed.
    Vector3 mFinalPosition;

    // The facing direction to use in the TurnToFace state.
    Vector3 mTurnToFaceDir = Vector3::UnitX;

    // A target (e.g. model) that we are walking to see when using "Walk To See".
    GKObject* mWalkToSeeTarget = nullptr;

    // A callback for when the current walk plan has completed (we've reached our destination).
    std::function<void()> mFinishedPathCallback = nullptr;

    // Is this a walk command from an autoscript?
    // If true, we tell the animation system this is an autoscript animation, and we don't restart idle autoscript when done.
    bool mFromAutoscript = false;

    // If true, need to continue walk anim during next update loop.
    bool mNeedContinueWalkAnim = false;

    // REGION SUPPORT
    // A callback for exiting a region.
    int mExitRegionIndex = -1;
    std::function<void()> mExitRegionCallback = nullptr;

    void WalkToInternal(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback, bool fromAutoscript, bool mustReachDestination);

    void PopAndNextAction();
    void NextAction();

    WalkOp GetCurrentWalkOp() const { return mWalkActions.empty() ? WalkOp::None : mWalkActions.back(); }
    void OnWalkToFinished();

    bool IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir) const;
    bool IsWalkToSeeTargetInView(const Vector3& headPosition, Vector3& outTurnToFaceDir) const;
    int FindPathNodeWhereWalkToSeeIsInView(Vector3& outInViewPos, Vector3& outTurnToFaceDir);

    int FindEarliestPathNodeInsideActiveTriggerRegion(Vector3& outEnterTriggerRegionPos);
    bool SkipPathNodesOutsideFrustum();

    bool AdvancePath();

    float GetWalkTurnSpeed(const Vector3& toNext);
    bool TurnToFace(float deltaTime, const Vector3& desiredDir, float turnSpeed);

    void PlayWalkLoopAnim(bool fromWalkStart);
    void StopAllWalkAnimations();

    void OutputWalkerPlan();
};