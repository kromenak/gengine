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
class Heading;
class WalkerBoundary;

class Walker : public Component
{
	TYPE_DECL_CHILD();
public:
	enum class State
	{
		Idle,
		Start,
		Loop,
		End
	};
	
	Walker(Actor* owner);
	
	void SetCharacterConfig(const CharacterConfig& characterConfig);
	
	void SnapToFloor();
	
	bool WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	bool WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	bool WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
	bool IsWalking() const { return mState != State::Idle || mHasDesiredFacingDir; }
	Vector3 GetDestination() const { return mDestination; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	const float kAtNodeDistSq = 100.0f;
	const float kAtHeadingRadians = Math::ToRadians(4.0f);
	
	// Owner (as GKActor) of this walker.
	GKActor* mGKOwner = nullptr;
	
	// Current state of the walker.
	State mState = State::Idle;
	
	// Config is vital for walker to function - contains things like
	// walk anims and hip position data.
	const CharacterConfig* mCharConfig = nullptr;
	
	// The path to follow to destination.
	std::vector<Vector3> mPath;
	
	// The current destination - only valid if walking.
	Vector3 mDestination;
	
	// If desired, a facing direction to move towards.
	// This only occurs *after* any path has been completed.
	bool mHasDesiredFacingDir = false;
	Vector3 mDesiredFacingDir;
	
	// A target (e.g. model name) that we are walking to see.
	// If set, the walker will end walking prematurely when it detects that the target model is in view.
	// The idea is that the walker doesn't need to walk right next to some things - it makes sense to stop when it comes into view.
	std::string mWalkToSeeTarget;
	Vector3 mWalkToSeeTargetPosition;
	
	// Turn speeds. A faster speed is used for turning in place when not walking.
	const float kWalkTurnSpeed = Math::kPi;
	const float kTurnSpeed = Math::k2Pi * 2;
	
	// A callback for when the end of a path is reached.
	std::function<void()> mFinishedPathCallback = nullptr;
	
	void StartWalk();
	void ContinueWalk();
	void StopWalk();
	
	void OnWalkToFinished();
	
	bool IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir);
};
