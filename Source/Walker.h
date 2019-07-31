//
// Walker.h
//
// Clark Kromenaker
//
// The walker (Texas Ranger) takes care of the intricacies of
// moving an actor's legs and walking from point to point in the game world.
//
#pragma once
#include "Component.h"

#include <functional>
#include <string>
#include <vector>

#include "GKActor.h"
#include "Vector3.h"

class Animation;
class CharacterConfig;
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
	
	void SetWalkMeshTransform(Transform* transform) { mWalkMeshTransform = transform; }
	void SetWalkAidMeshRenderer(MeshRenderer* aid) { mWalkAidMeshRenderer = aid; }
	
	bool IsWalking() const { return mState != State::Idle; }
	
	bool WalkTo(Vector3 position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	bool WalkTo(Vector3 position, float heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// Current state of the walker.
	State mState = State::Idle;
	
	// Animations to play while wsalking.
	Animation* mWalkStartAnim = nullptr;
	Animation* mWalkLoopAnim = nullptr;
	Animation* mWalkStopAnim = nullptr;
	
	/*
	 GK3 walk anims have translation baked in, rather than being centered/locked
	 at the origin. This makes it difficult to align the anim with the position
	 of the actor in the world.
	 
	 I'm not 100% sure how the original game resolved this - the GK3 postmortem
	 mentions this system is quite buggy!
	 
	 From what I can gather, an "aid" mesh animation is used to solve this problem.
	 The aid animation matches the walk anim, but in the opposite direction of
	 the actual walk animation. You can then offset the walking model by the offset
	 of the aid from the local origin to keep the walk animation in the right spot.
	*/
	MeshRenderer* mWalkAidMeshRenderer = nullptr;
	Transform* mWalkMeshTransform = nullptr;
	
	// The path to follow to destination.
	std::vector<Vector3> mPath;
	
	// If desired, a facing direction to move towards.
	// This only occurs *after* any path has been completed.
	bool mHasDesiredFacingDir = false;
	Vector3 mDesiredFacingDir;
	
	// Move and rotate speeds.
	float mMoveSpeed = 35.0f;
	float mRotateSpeed = 4.0f;
	
	// A callback for when the end of a path is reached.
	std::function<void()> mFinishedPathCallback = nullptr;
	
	void StartWalk();
	void ContinueWalk();
	void StopWalk();
};
