//
// GKActor.cpp
//
// Clark Kromenaker
//
// An "actor" in the GK3 system. An actor is considered to be
// one of the characters - like Gabe, Grace, Mosely. An actor is an entity
// that can walk around, animate, perform facial expressions and eye contact,
// play sound effects, etc.
//
// There are definitely "tiers" of actors - like, Gabe is an actor, but so
// are the "cat" and "chicken". So, maybe we need a subclass for humanoids at some point.
//
#pragma once
#include "Actor.h"

#include <string>

class Animation;
class GAS;
class GasPlayer;
class MeshRenderer;
class VertexAnimation;
class VertexAnimationPlayer;

class GKActor : public Actor
{
public:
    enum class State
    {
        Idle,
        Talk,
        Listen
    };
    
    GKActor(bool forCharacter);
	
	void PlayAnimation(VertexAnimation* animation);
	void PlayAnimation(VertexAnimation* animation, int framesPerSecond);
	
	void PlayAnimation(Animation* animation);
	void PlayInitAnimation(Animation* animation);
	
	void SetIdentifier(std::string identifier) { mIdentifier = identifier; }
	
	void SetNoun(std::string noun) { mNoun = noun; }
	std::string GetNoun() const { return mNoun; }
	
	void SetState(State state);
	
    void SetIdleGas(GAS* gas) { mIdleGas = gas; }
    void SetTalkGas(GAS* gas) { mTalkGas = gas; }
    void SetListenGas(GAS* gas) { mListenGas = gas; }
	
    MeshRenderer* GetMeshRenderer() const { return mMeshRenderer; }
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// The character's 3-letter identifier (GAB, GRA, etc).
	std::string mIdentifier;
	
	// This character's noun.
	std::string mNoun;
	
    // Actor's current state.
    State mState = State::Idle;
	
	// The character's mesh renderer.
	MeshRenderer* mMeshRenderer = nullptr;
	
	// The character's animation player.
	VertexAnimationPlayer* mAnimationPlayer = nullptr;
	
	// Player for GAS logic.
	GasPlayer* mGasPlayer = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleGas = nullptr;
    GAS* mTalkGas = nullptr;
    GAS* mListenGas = nullptr;
};
