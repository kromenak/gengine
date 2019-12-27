//
// Animator.cpp
//
// Clark Kromenaker
//
#include "Animator.h"

#include "Animation.h"
#include "AnimationNodes.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "VertexAnimation.h"

TYPE_DEF_CHILD(Component, Animator);

Animator::Animator(Actor* owner) : Component(owner)
{
    
}

void Animator::Start(Animation* animation, bool allowMove, std::function<void()> finishCallback)
{
	if(animation == nullptr) { return; }
	mActiveAnimations.emplace_back(animation, finishCallback);
	mActiveAnimations.back().allowMove = allowMove;
}

void Animator::Loop(Animation* animation)
{
	if(animation == nullptr) { return; }
	mActiveAnimations.emplace_back(animation);
	mActiveAnimations.back().loop = true;
}

void Animator::Stop(Animation* animation)
{
	if(animation == nullptr) { return; }
	auto newEndIt = std::remove_if(mActiveAnimations.begin(), mActiveAnimations.end(), [animation](const AnimationState& as) -> bool {
		if(as.animation == animation)
		{
			// If stopping an animation, be sure to also stop any running vertex animations.
			auto& vertexAnims = animation->GetVertexAnimNodes();
			for(auto& vertexAnim : vertexAnims)
			{
				if(vertexAnim->frameNumber <= as.currentFrame)
				{
					vertexAnim->Stop();
				}
			}
			return true;
		}
		return false;
	});
	mActiveAnimations.erase(newEndIt, mActiveAnimations.end());
}

void Animator::Sample(Animation* animation, int frame)
{
	if(animation == nullptr) { return; }
	
	// Sample any anim nodes for the desired frame.
	std::vector<AnimNode*>* frameData = animation->GetFrame(frame);
	if(frameData != nullptr)
	{
		for(auto& node : *frameData)
		{
			node->Sample(animation, frame);
		}
	}
}

void Animator::OnUpdate(float deltaTime)
{
	// Iterate over each active animation state and update it.
	auto it = mActiveAnimations.begin();
	while(it != mActiveAnimations.end())
	{
		AnimationState& animState = *it;
		
		// Increment animation timer.
		animState.timer += deltaTime;
		
		// Based on the animation's frames per second, determine what
		// fraction of a second should be spend on each frame.
		float timePerFrame = 1.0f / animState.animation->GetFramesPerSecond();
		
		// Based on how much time has passed, we may need to increment multiple frames of animation in one update loop.
		// For example, if timer is 0.3, and timePerFrame is 0.1, we need to update 3 times.
		while(animState.timer > timePerFrame)
		{
			// Play any anim nodes for the current frame.
			std::vector<AnimNode*>* frameData = animState.animation->GetFrame(animState.currentFrame);
			if(frameData != nullptr)
			{
				for(auto& node : *frameData)
				{
					node->Play(&animState);
				}
			}
			
			// Move on to the next frame.
			animState.currentFrame++;
			
			// If looping, wrap around the current frame when we reach the end!
			if(animState.loop)
			{
				animState.currentFrame %= animState.animation->GetFrameCount();
			}
			
			// Decrement timer, since we just simulated a single frame.
			animState.timer -= timePerFrame;
		}
		
		// If the animation has ended, remove it from the active animation states.
		if(animState.currentFrame >= animState.animation->GetFrameCount())
		{
			it = mActiveAnimations.erase(it);
			
			// Do the finish callback!
			if(animState.finishCallback)
			{
				animState.finishCallback();
			}
		}
		else
		{
			++it;
		}
	}
}
