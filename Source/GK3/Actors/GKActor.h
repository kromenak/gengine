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

#include "Heading.h"

struct CharacterConfig;
class FaceController;
class GAS;
class VertexAnimation;
class VertexAnimator;
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
	
	// Constructors
	GKActor();
	GKActor(const std::string& identifier);
	
	void SetHeading(const Heading& heading) override;
	
	///
	/// ACTOR AND PROP FUNCTIONS
	///
	std::string GetModelName() const;
	
	MeshRenderer* GetMeshRenderer() const { return mMeshRenderer; }
	VertexAnimator* GetVertexAnimator() const { return mVertexAnimator; }
	GasPlayer* GetGasPlayer() const { return mGasPlayer; }
	
	// Vertex animation functions.
	void StartAnimation(VertexAnimation* anim, int framesPerSecond, bool allowMove, float time, bool fromGas);
	void StartAbsoluteAnimation(VertexAnimation* anim, int framesPerSecond, Vector3 pos, Heading heading, float time, bool fromGas);
	void StopAnimation(VertexAnimation* anim = nullptr);
	void SampleAnimation(VertexAnimation* anim, int frame);
	
	///
	/// ACTOR ONLY FUNCTIONS
	///
	const std::string& GetIdentifier() const { return mIdentifier; }
	
    void SetIdleFidget(GAS* fidget) { mIdleFidget = fidget; }
    void SetTalkFidget(GAS* fidget) { mTalkFidget = fidget; }
    void SetListenFidget(GAS* fidget) { mListenFidget = fidget; }
	
	void StartFidget(FidgetType type);
	void StartCustomFidget(GAS* gas);
	void StopFidget();
	
	void TurnTo(const Heading& heading, std::function<void()> finishCallback);
	
	void WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkToAnimationStart(Animation* anim, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	void WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	Vector3 GetWalkDestination() const;
	
	void SnapToFloor();
	
	FaceController* GetFaceController() const { return mFaceController; }
    
    void SetWalkerDOR(GKActor* walkerDOR) { mWalkerDOR = walkerDOR; }
    
	Vector3 GetHeadPosition() const;
	
	void DumpPosition();
	
protected:
	void OnActive() override;
	void OnInactive() override;
	void OnUpdate(float deltaTime) override;
	
private:
	enum class ActorType
	{
		Actor,
		Prop
	};
	ActorType mActorType = ActorType::Actor;
	
	///
	/// ACTOR AND PROP VARIABLES
	///
	// Mesh is attached to a separate actor so that the mesh can move separately from the rest of the actor.
	// This is sometimes necessary for how GK3 does walking and vertex animations.
	Actor* mMeshActor = nullptr;
	
	// Allows the object to render a 3D mesh.
	MeshRenderer* mMeshRenderer = nullptr;
	
	// Many objects animate using vertex animations.
	VertexAnimator* mVertexAnimator = nullptr;
	
	// GAS player allows object to animate in an automated/scripted fashion based on some simple command statements.
	// Used by both actors and props.
	GasPlayer* mGasPlayer = nullptr;
	
	// Vertex anims often change the position of the mesh, but that doesn't mean the actor's position should change.
	// Sometimes we allow a vertex anim to affect the actor's position.
	bool mVertexAnimAllowMove = false;
	Vector3 mStartVertexAnimPosition;
	Quaternion mStartVertexAnimRotation;
    
    Vector3 mStartVertexAnimMeshPos;
    Quaternion mStartVertexAnimMeshRotation;
    
    Vector3 mLastMeshPos;
    Quaternion mLastMeshRotation;
	
	///
	/// ACTOR ONLY VARIABLES
	///
	// The actor's 3-letter identifier (GAB, GRA, etc).
	// Every GK Actor has one. Note this is NOT THE SAME as the noun!
	std::string mIdentifier;
	
	// The character's configuration, which defines helpful parameters for controlling the actor.
	const CharacterConfig* mCharConfig = nullptr;
	
	// The actor's walking control.
	Walker* mWalker = nullptr;
    GKActor* mWalkerDOR = nullptr;
	
	// The actor's face control.
	FaceController* mFaceController = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
	FidgetType mCurrentFidget = FidgetType::None;
    GAS* mIdleFidget = nullptr;
    GAS* mTalkFidget = nullptr;
    GAS* mListenFidget = nullptr;
	
	void OnVertexAnimationStopped();
    
    void SyncMeshTransformToActor(VertexAnimation* anim = nullptr);
    void SyncActorTransformToMesh();
    
    Vector3 GetMeshPosition();
    Quaternion GetMeshRotation();
};
