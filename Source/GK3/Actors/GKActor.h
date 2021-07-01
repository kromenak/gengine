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
#include "Walker.h"

struct CharacterConfig;
class FaceController;
class GAS;
class Model;
class VertexAnimation;
class VertexAnimator;
struct VertexAnimParams;

class GKActor : public GKProp
{
public:
	GKActor(Model* model);
	
    enum class FidgetType
    {
        None,
        Idle,
        Talk,
        Listen
    };
	void StartFidget(FidgetType type);
    void StopFidget(std::function<void()> callback = nullptr) override;

    void SetIdleFidget(GAS* fidget);
    void SetTalkFidget(GAS* fidget);
    void SetListenFidget(GAS* fidget);
	
	void TurnTo(const Heading& heading, std::function<void()> finishCallback);
    
	void WalkTo(const Vector3& position, const Heading& heading, std::function<void()> finishCallback);
	void WalkTo(const Vector3& position, std::function<void()> finishCallback);
	void WalkToAnimationStart(Animation* anim, std::function<void()> finishCallback);
	void WalkToSee(const std::string& targetName, const Vector3& targetPosition, std::function<void()> finishCallback);

    void SetWalkerBoundary(WalkerBoundary* walkerBoundary);
    Vector3 GetWalkDestination() const;
    bool AtPosition(const Vector3& position) { return mWalker->AtPosition(position); }
    void SetWalkerDOR(GKProp* walkerDOR);
    void SnapToFloor();
    
	FaceController* GetFaceController() const { return mFaceController; }
	Vector3 GetHeadPosition() const;
    
    void SetPosition(const Vector3& position);
    void SetHeading(const Heading& heading) override;
	
protected:
	void OnUpdate(float deltaTime) override;
    
    void OnVertexAnimationStart(const VertexAnimParams& animParams) override;
    void OnVertexAnimationStop() override;
    
private:
	// The character's configuration, which defines helpful parameters for controlling the actor.
	const CharacterConfig* mCharConfig = nullptr;

    // The fidget the actor is currently playing.
    FidgetType mActiveFidget = FidgetType::None;
	
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
    
    // For non-move vertex anims, the actor resets to its original position/rotation when the anim stops.
    // So, we must save the start position/rotation for that purpose.
    Vector3 mStartVertexAnimPosition;
    Quaternion mStartVertexAnimRotation;
    
    //Vector3 mStartVertexAnimModelPosition;
    //Quaternion mStartVertexAnimModelRotation;
    
    // Usually, the model drives the actor's position (think: root motion).
    // To do that, we track the models last position/rotation and move the actor every frame to keep up.
    Vector3 mLastModelPosition;
    Quaternion mLastModelRotation;
    
    Vector3 GetModelPosition();
    Quaternion GetModelRotation();
    
    void SyncModelToActor();
    void SyncActorToModel();

    GAS* GetGasForFidget(FidgetType type);
    void CheckUpdateActiveFidget(FidgetType changedType);
};
