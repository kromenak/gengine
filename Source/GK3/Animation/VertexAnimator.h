//
// Clark Kromenaker
//
// A component that can play vertex animations on an actor.
//
#pragma once
#include "Component.h"

#include <functional>

#include "Heading.h"
#include "Profiler.h" // For Stopwatch
#include "Vector3.h"

class MeshRenderer;
class VertexAnimation;

struct VertexAnimParams
{
	// The anim to play.
	VertexAnimation* vertexAnimation = nullptr;
	
	// Rate to play the animation at.
	int framesPerSecond = 15;
	
	// Time to start the animation at.
	float startTime = 0.0f;
    //TODO: startFrame?
    
    // An absolute anim plays from a specific position/rotation
    // (in contrast to a "relative" anim that plays from model's last position/rotation).
    bool absolute = false;
    Vector3 absolutePosition;
    Heading absoluteHeading = Heading::None;

    // If specified, it means this anim follows a parent object's movement. Should not be set for absolute anims.
    Actor* parent = nullptr;
    
    // If true, animation can move associated character (kind of like "root motion").
    bool allowMove = false;
    
    // If true, this anim was started from an autoscript (GAS).
    bool fromAutoScript = false;
    
	// A callback to fire on animation stop.
	std::function<void()> stopCallback = nullptr;
};

class VertexAnimator : public Component
{
	TYPE_DECL_CHILD();
public:
	VertexAnimator(Actor* owner);
	
    void Start(const VertexAnimParams& params);
	void Stop(VertexAnimation* anim = nullptr);
	
	void Sample(VertexAnimation* animation, int frame);
	
	bool IsPlaying() const { return mCurrentParams.vertexAnimation != nullptr; }
    bool IsPlayingNotAutoscript() const { return mCurrentParams.vertexAnimation != nullptr && !mCurrentParams.fromAutoScript; }
	
protected:
    void OnEnable() override;
    void OnDisable() override;
	void OnUpdate(float deltaTime) override;
	
private:
	// The mesh renderer that will be animated.
	MeshRenderer* mMeshRenderer = nullptr;

    // Params for current animation (if any).
    // Currently running animation, FPS, stop callback, parent - all stored in here!
    VertexAnimParams mCurrentParams;
	
	// Timer for tracking progress on vertex animation.
	float mAnimationTimer = 0.0f;

    // Problem: GK3 assumes objects continue to animate when they are not visible. But inactive objects don't Update!
    // To work around that, we'll use this timer to track how long a VertexAnimator is disabled.
    Stopwatch mDisabledTimer;
	
    void TakeSample(VertexAnimation* animation, int frame);
	void TakeSample(VertexAnimation* animation, float time);
};
