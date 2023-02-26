//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of moving an actor's legs
// and walking from point to point in the game world.
//
// A walker is always associated with a GKActor.
//
#pragma once
#include "Component.h"

#include <functional>
#include <string>
#include <vector>

#include "Vector3.h"

class Animation;
struct CharacterConfig;
class GKActor;
class GKObject;
class GKProp;
class Heading;
class Texture;
class WalkerBoundary;

class Walker : public Component
{
	TYPE_DECL_CHILD();
public:
	Walker(Actor* owner);

    void SetWalkerBoundary(WalkerBoundary* walkerBoundary) { mWalkerBoundary = walkerBoundary; }
    void SetCharacterConfig(const CharacterConfig& characterConfig);
    void SetWalkAnims(Animation* startAnim, Animation* loopAnim,
                      Animation* startTurnLeftAnim, Animation* startTurnRightAnim);

	void WalkTo(const Vector3& position, std::function<void()> finishCallback);
	void WalkTo(const Vector3& position, const Heading& heading, std::function<void()> finishCallback);
    void WalkToGas(const Vector3& position, const Heading& heading, std::function<void()> finishCallback);
    void WalkToSee(GKObject* target, std::function<void()> finishCallback);

    void WalkOutOfRegion(int regionIndex, const Vector3& exitPosition, const Heading& exitHeading, std::function<void()> finishCallback);

    void SkipToEnd();

    bool AtPosition(const Vector3& position);
    bool IsWalking() const { return mWalkActions.size() > 0; }
    Vector3 GetDestination() const { return mPath.size() > 0 ? mPath.front() : Vector3::Zero; }

    Texture* GetFloorTypeWalkingOn() const;

protected:
	void OnUpdate(float deltaTime) override;
	
private:
    // When this close to a position/heading, we say you are "at" the position/heading.
    const float kAtNodeDist = 12.25f;
	const float kAtNodeDistSq = kAtNodeDist * kAtNodeDist;
	const float kAtHeadingRadians = Math::ToRadians(4.0f);
    
    // Turn speeds. A faster speed is used for turning in place when not walking.
    const float kWalkTurnSpeed = Math::kPi;
    const float kTurnSpeed = Math::k2Pi;

    // When a walk begins, a "plan" is generated and stored in the walk actions vector.
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
    struct WalkAction
    {
        WalkOp op;
        Vector3 facingDir;
    };
    std::vector<WalkAction> mWalkActions;  // Walk actions are in reverse - next action to perform is back().
    
    // In at least one case, it's helpful to know what the previous walk operation was.
    WalkOp mPrevWalkOp = WalkOp::None;
    
    // The amount of time spent on the current walk action.
    float mCurrentWalkActionTimer = 0.0f;
	
	// Walker's owner, as a GKActor.
	GKActor* mGKOwner = nullptr;

    // Config is vital for walker to function - contains things like walk anims and hip position data.
    const CharacterConfig* mCharConfig = nullptr;

    // Walk animations currently in use.
    // Need to store separate from CharacterConfig because they can be overridden in some circumstances.
    Animation* mWalkStartAnim = nullptr;
    Animation* mWalkLoopAnim = nullptr;
    Animation* mWalkStartTurnLeftAnim = nullptr;
    Animation* mWalkStartTurnRightAnim = nullptr;

    // Walker boundary currently being used.
    WalkerBoundary* mWalkerBoundary = nullptr;
    
	// The path to follow to destination.
    // Note that this path is BACKWARDS - the goal is front() and next node to go to is back().
	std::vector<Vector3> mPath;
	
	// A target (e.g. model) that we are walking to see.
    GKObject* mWalkToSeeTarget = nullptr;

    // A callback for exiting a region.
    int mExitRegionIndex = -1;
    std::function<void()> mExitRegionCallback = nullptr;

	// A callback for when the end of a path is reached.
	std::function<void()> mFinishedPathCallback = nullptr;

    // Is this a walk command from an autoscript?
    // If true, we tell the animation system this is an autoscript animation, and we don't restart idle autoscript when done.
    bool mFromAutoscript = false;

    // If true, need to continue walk anim during next update loop.
    bool mNeedContinueWalkAnim = false;

    void WalkToInternal(const Vector3& position, const Heading& heading, std::function<void()> finishCallback, bool fromAutoscript);

    void PopAndNextAction();
    void NextAction();
    
    bool IsMidWalk() const { return mWalkActions.size() > 0 && mWalkActions.back().op == WalkOp::FollowPath; }
	void OnWalkAnimFinished();
	void OnWalkToFinished();
	
	bool IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir);
    
    bool CalculatePath(const Vector3& startPos, const Vector3& endPos);
    bool AdvancePath();
    
    bool TurnToFace(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir = 0);
};
