//
// Walker.h
//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of moving an actor's legs
// and walking from point to point in the game world.
//
// A walker is always associated with a GKActor.
// The walker moves along a path using A*.
//
#pragma once
#include "Component.h"

#include <functional>
#include <string>
#include <vector>

#include "GKActor.h"
#include "Vector3.h"

class Animation;
struct CharacterConfig;
class GKProp;
class Heading;
class WalkerBoundary;

class Walker : public Component
{
	TYPE_DECL_CHILD();
public:
	Walker(Actor* owner);
	
    void SetCharacterConfig(const CharacterConfig& characterConfig) { mCharConfig = &characterConfig; }
    void SetHeadingProp(GKProp* headingProp) { mHeadingProp = headingProp; }
    
	void SnapToFloor();
	
	void WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
    void WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
    bool IsWalking() const { return mWalkActions.size() > 0; }
    Vector3 GetDestination() const { return mPath.size() > 0 ? mPath.front() : mGKOwner->GetPosition(); }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	const float kAtNodeDistSq = 150.0f;
	const float kAtHeadingRadians = Math::ToRadians(4.0f);
    
    // Turn speeds. A faster speed is used for turning in place when not walking.
    const float kWalkTurnSpeed = Math::kPi;
    const float kTurnSpeed = Math::k2Pi;
    
    enum class WalkOp
    {
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
    std::vector<WalkAction> mWalkActions;
	
	// Walker component is attached to GKActor.
    // Remember that the actual animating mesh is a separate actor!
	GKActor* mGKOwner = nullptr;
    
    GKProp* mHeadingProp = nullptr;
	
	// Config is vital for walker to function - contains things like
	// walk anims and hip position data.
	const CharacterConfig* mCharConfig = nullptr;
	
    // Walker boundary last used.
    WalkerBoundary* mWalkerBoundary = nullptr;
    
	// The path to follow to destination.
    // Note that this path is BACKWARDS - the goal is front() and next node to go to is back().
	std::vector<Vector3> mPath;
	
	// A target (e.g. model name) that we are walking to see.
	// If set, the walker will end walking prematurely when it detects that the target model is in view.
	// The idea is that the walker doesn't need to walk right next to some things - it makes sense to stop when it comes into view.
	std::string mWalkToSeeTarget;
	Vector3 mWalkToSeeTargetPosition;
	
	// A callback for when the end of a path is reached.
	std::function<void()> mFinishedPathCallback = nullptr;
	
    void PopAndNextAction();
    void NextAction();
    
	void ContinueWalk();
	
	void OnWalkToFinished();
	
	bool IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir);
    
    bool CalculatePath(const Vector3& startPos, const Vector3& endPos);
    
    bool AdvancePath();
    bool TurnToFace(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir = 0, bool aboutOwnerPos = true);
    Quaternion GetTurnAmount(float deltaTime, const Vector3& currentDir, const Vector3& desiredDir, float turnSpeed, int turnDir = 0);
    
    Vector3 GetHeadingDir();
};
