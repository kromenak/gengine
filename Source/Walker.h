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

#include <string>
#include <vector>

#include "Vector3.h"

class Animation;
class CharacterConfig;

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
	void SetPath(std::vector<Vector3> path) { mPath = path; StartWalk(); }
	
	bool IsWalking() const { return mState != State::Idle; }
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// Current state of the walker.
	State mState = State::Idle;
	
	// Animations to play while walking.
	Animation* mWalkStartAnim = nullptr;
	Animation* mWalkLoopAnim = nullptr;
	Animation* mWalkStopAnim = nullptr;
	
	// The path to follow to destination.
	std::vector<Vector3> mPath;
	
	void StartWalk();
	void ContinueWalk();
	void StopWalk();
};
