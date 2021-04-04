//
// Animator.cpp
//
// Clark Kromenaker
//
// Plays animations (ANM assets)!
//
// This is more of a "manager" style class than a per-actor instance class.
// See VertexAnimator for actor instance vertex animation logic.
//
#pragma once
#include "Component.h"

#include <functional>
#include <list>

class Animation;
class VertexAnimation;
class MeshRenderer;

struct AnimationState
{
	// The animation that is playing.
	Animation* animation = nullptr;
	
	// The current frame in the animation.
	int currentFrame = 0;
	
	// A timer to track when we need to execute one or more additional frames.
	// This doesn't track total animation time, just time until the next frame!
	float timer = 0.0f;
	
	// If true, the animation loops!
	bool loop = false;
	
	// If true, the animation is allowed to move actors as part of vertex animations.
	bool allowMove = false;
	
	// If true, this is an animation triggered by a GAS (GK3 auto-script).
	// This mainly indicates that the animation is lower-priority than other anims.
	bool fromGas = false;
    
    // If true, this is a "yak" animation - which mainly indicates that audio should play as VO.
    bool isYak = false;
	
	// Callback that is executed when the animation finishes.
	//TODO: What about premature stops?
	std::function<void()> finishCallback = nullptr;
    
    // Needed for "emplace" usage.
    AnimationState(Animation* animation) : animation(animation) { }
    AnimationState(Animation* animation, std::function<void()> finishCallback) :
        animation(animation), finishCallback(finishCallback) { }
    
    void Stop();
};

class Animator : public Component
{
    TYPE_DECL_CHILD();
public:
    Animator(Actor* owner);
	
	// Playback
	void Start(Animation* animation, bool allowMove = false, bool fromGas = false, std::function<void()> finishCallback = nullptr);
    void StartYak(Animation* yakAnimation, std::function<void()> finishCallback = nullptr);
    void Loop(Animation* animation);
	void Stop(Animation* animation);
    void StopAll();
    
	// Sampling
	void Sample(Animation* animation, int frame);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// I chose to use a linked list here b/c we may need to remove animations that are
	// in the middle of the list at arbitrary times...not sure if the list is big enough or we do it often enough to get benefits?
	std::list<AnimationState> mActiveAnimations;
	
	void ExecuteFrame(AnimationState& animState, int frameNumber);
};
