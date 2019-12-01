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
#include "GKObject.h"

#include <string>

class FaceController;
class GAS;
class VertexAnimation;
class Walker;

class GKActor : public GKObject
{
public:
    enum class FidgetType
    {
		None,
        Idle,
        Talk,
        Listen,
		Custom
    };
	
	GKActor(const std::string& identifier);
	
    void SetIdleGas(GAS* gas) { mIdleGas = gas; }
    void SetTalkGas(GAS* gas) { mTalkGas = gas; }
    void SetListenGas(GAS* gas) { mListenGas = gas; }
	
	void StartFidget(FidgetType type);
	void StartCustomFidget(GAS* gas);
	
	const std::string& GetIdentifier() const { return mIdentifier; }
	
	Walker* GetWalker() const { return mWalker; }
	FaceController* GetFaceController() const { return mFaceController; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// The actor's 3-letter identifier (GAB, GRA, etc).
	// Every GK Actor has one. Note this is NOT THE SAME as the noun!
	std::string mIdentifier;
	
    // Actor's current fidget.
    FidgetType mActiveFidget = FidgetType::Idle;
	
	// The actor's walking control.
	Walker* mWalker = nullptr;
	
	// The actor's face control.
	FaceController* mFaceController = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleGas = nullptr;
    GAS* mTalkGas = nullptr;
    GAS* mListenGas = nullptr;
};
