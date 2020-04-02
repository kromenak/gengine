//
// Walker.h
//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of moving an actor's legs
// and walking from point to point in the game world.
//
// A walker is always associated with a GKActor, but it is also a completely separate Actor in the world.
// The walker moves along a path using A*. It then also updates the actor it's associated with.
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
	void SetWalkMeshActor(Actor* walkMeshActor) { mWalkMeshActor = walkMeshActor; }
	
	void SnapWalkActorToFloor();
	
	bool IsWalking() const { return mState != State::Idle || mHasDesiredFacingDir; }
	
	bool WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	bool WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	const float kAtNodeDistSq = 100.0f;
	const float kAtHeadingRadians = Math::ToRadians(5.0f);
	
	// Current state of the walker.
	State mState = State::Idle;
	
	// Config is vital for walker to function - contains things like
	// walk anims and hip position data.
	const CharacterConfig* mCharConfig = nullptr;
	
	// Actor who is driven by the walker.
	Actor* mWalkMeshActor = nullptr;
	
	// The path to follow to destination.
	std::vector<Vector3> mPath;
	
	// If desired, a facing direction to move towards.
	// This only occurs *after* any path has been completed.
	bool mHasDesiredFacingDir = false;
	Vector3 mDesiredFacingDir;
	
	// Move and rotate speeds.
	// Why these values? Trial and error...
	const float mMoveSpeed = 35.0f;
	const float mRotateSpeed = Math::kPi;
	
	// A callback for when the end of a path is reached.
	std::function<void()> mFinishedPathCallback = nullptr;
	
	void StartWalk();
	void ContinueWalk();
	void StopWalk();
};
