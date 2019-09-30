//
// VertexAnimationPlayer.h
//
// Clark Kromenaker
//
// A component that can play vertex animations on an actor.
//
#pragma once
#include "Component.h"

class MeshRenderer;
class VertexAnimation;

class VertexAnimationPlayer : public Component
{
	TYPE_DECL_CHILD();
public:
	VertexAnimationPlayer(Actor* owner);
	
	void Play(VertexAnimation* animation);
	void Play(VertexAnimation* animation, int framesPerSecond);
	
	void Sample(VertexAnimation* animation, int frame);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// The mesh renderer from which we will animate.
	MeshRenderer* mMeshRenderer = nullptr;
	
	// How many frames per second to run at. Default is 15 (from GK3 docs).
	int mFramesPerSecond = 15;
	
	// If defined, a currently running vertex animation.
	VertexAnimation* mVertexAnimation = nullptr;
	
	// Timer for tracking progress on vertex animation.
	float mVertexAnimationTimer = 0.0f;
	
	void TakeSample(VertexAnimation* animation, float time);
};
