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
// You can think of a "GK actor" as a more complex prop - it does what they do, and more.
// The inheritance hierarchy reflects this.
//
#pragma once
#include "GKProp.h"

#include <string>

#include "Heading.h"

struct CharacterConfig;
class FaceController;
class GAS;
class VertexAnimation;
class VertexAnimator;
struct VertexAnimParams;
class Walker;

class GKActor : public GKProp
{
public:
	GKActor(const std::string& identifier);
	
	const std::string& GetIdentifier() const { return mIdentifier; }
	
    void SetIdleFidget(GAS* fidget) { mIdleFidget = fidget; }
    void SetTalkFidget(GAS* fidget) { mTalkFidget = fidget; }
    void SetListenFidget(GAS* fidget) { mListenFidget = fidget; }
	
    enum class FidgetType
    {
        Idle,
        Talk,
        Listen
    };
	void StartFidget(FidgetType type);
	
	void TurnTo(const Heading& heading, std::function<void()> finishCallback);
    
	void WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkToAnimationStart(Animation* anim, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
    Vector3 GetWalkDestination() const;
    void SetWalkerDOR(GKProp* walkerDOR);
	void SnapToFloor();
    
	FaceController* GetFaceController() const { return mFaceController; }
	Vector3 GetHeadPosition() const;
	
protected:
	void OnUpdate(float deltaTime) override;
    
    void OnVertexAnimationStart(const VertexAnimParams& animParams) override;
    void OnVertexAnimationStop() override;
    
private:
	// The actor's 3-letter identifier (GAB, GRA). Matches model name by convention (GAB.MOD, GRA.MOD).
	// Note this is NOT THE SAME as the noun (GABRIEL, GRACE).
	std::string mIdentifier;
	
	// The character's configuration, which defines helpful parameters for controlling the actor.
	const CharacterConfig* mCharConfig = nullptr;
	
	// The actor's walking control.
	Walker* mWalker = nullptr;
    GKProp* mWalkerDOR = nullptr;
	
	// The actor's face control.
	FaceController* mFaceController = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleFidget = nullptr;
    GAS* mTalkFidget = nullptr;
    GAS* mListenFidget = nullptr;
    
    // Vertex anims often change the position of the mesh, but that doesn't mean the actor's position should change.
    // Sometimes we allow a vertex anim to affect the actor's position.
    bool mVertexAnimAllowMove = false;
    
    Vector3 mStartVertexAnimPosition;
    Quaternion mStartVertexAnimRotation;
    
    Vector3 mStartVertexAnimMeshPos;
    Quaternion mStartVertexAnimMeshRotation;
    
    Vector3 mLastMeshPos;
    Quaternion mLastMeshRotation;
    
    Vector3 GetMeshPosition();
    Quaternion GetMeshRotation();
};
