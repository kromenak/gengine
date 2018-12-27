//
// AnimationPlayer.cpp
//
// Clark Kromenaker
//
// Plays animations!
//
#pragma once
#include "Component.h"

#include <list>

class Animation;
class VertexAnimation;
class MeshRenderer;

struct AnimationState
{
	// Needed for "emplace" usage.
	AnimationState(Animation* animation) : animation(animation) { }
	
	AnimationState(Animation* animation, std::function<void()> finishCallback) :
		animation(animation), finishCallback(finishCallback) { }
	
	// The animation that is playing.
	Animation* animation = nullptr;
	
	// The current frame in the animation.
	int currentFrame = 0;
	
	// A timer to track when we need to execute one or more additional frames.
	// This doesn't track total animation time, just time until the next frame!
	float timer = 0.0f;
	
	// Callback that is executed when the animation finishes.
	//TODO: What about premature stops?
	std::function<void()> finishCallback = nullptr;
};

class AnimationPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    AnimationPlayer(Actor* owner);
    
    void Play(Animation* animation);
	void Play(Animation* animation, std::function<void()> finishCallback);
	
	void Sample(Animation* animation, int frame);
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	std::list<AnimationState> mActiveAnimationStates;
};
