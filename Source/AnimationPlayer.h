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
	AnimationState(Animation* animation, int currentFrame, float timer) :
		animation(animation), currentFrame(currentFrame), timer(timer) { }
	
	Animation* animation = nullptr;
	int currentFrame = 0;
	float timer = 0.0f;
};

class AnimationPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    AnimationPlayer(Actor* owner);
    
    void Play(Animation* animation);
    void Play(VertexAnimation* vertexAnimation);
	
	void Sample(Animation* animation, int frame);
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
    // Needs a MeshRenderer to update the mesh data every frame.
    MeshRenderer* mMeshRenderer = nullptr;
    
    // The animation currently being played.
    Animation* mAnimation = nullptr;
	
    // The frame we're currently on in the animation.
    //int mCurrentAnimationFrame = 0;
    //float mAnimationTimer = 0.0f;
	
	std::list<AnimationState> mActiveAnimations;
    
    // If defined, a currently running vertex animation.
    //VertexAnimation* mVertexAnimation = nullptr;
    //float mVertexAnimationTimer = 0.0f;
};
