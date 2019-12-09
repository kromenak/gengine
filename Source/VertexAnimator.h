//
// VertexAnimator.h
//
// Clark Kromenaker
//
// A component that can play vertex animations on an actor.
//
#pragma once
#include "Component.h"

#include <functional>

class MeshRenderer;
class VertexAnimation;

class VertexAnimator : public Component
{
	TYPE_DECL_CHILD();
public:
	static const int kDefaultFramesPerSecond = 15;
	
	VertexAnimator(Actor* owner);
	
	void Start(VertexAnimation* anim, int framesPerSecond, std::function<void()> stopCallback);
	void Stop(VertexAnimation* anim);
	
	void Sample(VertexAnimation* animation, int frame);
	
	bool IsPlaying() const { return mVertexAnimation != nullptr; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// The mesh renderer that will be animated.
	MeshRenderer* mMeshRenderer = nullptr;
	
	// How many frames per second to run at. Default is 15 (from GK3 docs).
	int mFramesPerSecond = kDefaultFramesPerSecond;
	
	// A currently running vertex animation, if any.
	VertexAnimation* mVertexAnimation = nullptr;
	
	// Callback that is fired when the animation stops.
	// "Stops" means manually stopped OR reached end of playback!
	std::function<void()> mStopCallback = nullptr;
	
	// Timer for tracking progress on vertex animation.
	float mVertexAnimationTimer = 0.0f;
	
	void TakeSample(VertexAnimation* animation, float time);
};
