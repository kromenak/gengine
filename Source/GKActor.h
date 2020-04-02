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

#include "Heading.h"

struct CharacterConfig;
class FaceController;
class GAS;
class VertexAnimation;
class VertexAnimator;
class Walker;

class GKActor : public Actor
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
	
	///
	/// BASE GK3 OBJECT FUNCTIONS
	///
	// GK3 rotations are often defined in terms of a heading (360 degrees about Y-axis).
	void SetHeading(const Heading& heading);
	Heading GetHeading() const;
	
	///
	/// ACTOR AND PROP FUNCTIONS
	///
	void SetNoun(std::string noun) { mNoun = noun; }
	std::string GetNoun() const { return mNoun; }
	
	std::string GetModelName() const;
	
	MeshRenderer* GetMeshRenderer() const { return mMeshRenderer; }
	VertexAnimator* GetVertexAnimator() const { return mVertexAnimator; }
	GasPlayer* GetGasPlayer() const { return mGasPlayer; }
	
	// Vertex animation functions.
	void StartAnimation(VertexAnimation* anim, int framesPerSecond, bool allowMove);
	void StartAnimation(VertexAnimation* anim, int framesPerSecond, Vector3 pos, Heading heading);
	void StopAnimation(VertexAnimation* anim);
	void SampleAnimation(VertexAnimation* anim, int frame);
	
	///
	/// ACTOR ONLY FUNCTIONS
	///
	const std::string& GetIdentifier() const { return mIdentifier; }
	
    void SetIdleGas(GAS* gas) { mIdleGas = gas; }
    void SetTalkGas(GAS* gas) { mTalkGas = gas; }
    void SetListenGas(GAS* gas) { mListenGas = gas; }
	
	void StartFidget(FidgetType type);
	void StartCustomFidget(GAS* gas);
	
	void WalkToAnimationStart(Animation* anim, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback);
	
	Walker* GetWalker() const { return mWalker; }
	FaceController* GetFaceController() const { return mFaceController; }
	
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
	
	enum class AnimMode
	{
		None,
		Relative,
		Absolute,
		Walking
	};
	AnimMode mAnimMode = AnimMode::None;
	
	///
	/// ACTOR AND PROP VARIABLES
	///
	// The object's noun. Almost all props and actors have a noun that is used
	// to refer to the object in SIF/NVC/Sheep logic.
	std::string mNoun;
	
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
	
	// The actor's face control.
	FaceController* mFaceController = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleGas = nullptr;
    GAS* mTalkGas = nullptr;
    GAS* mListenGas = nullptr;
	
	void OnVertexAnimationStopped();
	
	void SetMeshToActorPosition(bool useMeshPosOffset);
	void SetMeshToActorPositionUsingAnim(VertexAnimation* anim, int framesPerSecond);
	void SetActorToMeshPosition(bool useMeshPosOffset);
	
	void SetMeshToActorRotation(bool useMeshPosOffset);
	void SetActorToMeshRotation(bool useMeshPosOffset);
};
