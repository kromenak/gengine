//
// AnimationPlayer.cpp
//
// Clark Kromenaker
//
#include "AnimationPlayer.h"

#include "Animation.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "VertexAnimation.h"

TYPE_DEF_CHILD(Component, AnimationPlayer);

AnimationPlayer::AnimationPlayer(Actor* owner) : Component(owner)
{
    
}

void AnimationPlayer::Play(Animation* animation)
{
	if(animation == nullptr) { return; }
	mActiveAnimationStates.emplace_back(animation);
}

void AnimationPlayer::Play(Animation* animation, std::function<void()> finishCallback)
{
	if(animation == nullptr) { return; }
	mActiveAnimationStates.emplace_back(animation, finishCallback);
}

void AnimationPlayer::Stop(Animation* animation)
{
	if(animation == nullptr) { return; }
	auto newEndIt = std::remove_if(mActiveAnimationStates.begin(), mActiveAnimationStates.end(), [animation](const AnimationState& as) -> bool {
		return as.animation == animation;
	});
	mActiveAnimationStates.erase(newEndIt, mActiveAnimationStates.end());
}

void AnimationPlayer::Sample(Animation* animation, int frame)
{
	if(animation == nullptr) { return; }
	
	// Play any anim nodes for the desired frame.
	std::vector<AnimNode*>* frameData = animation->GetFrame(frame);
	if(frameData != nullptr)
	{
		for(auto& node : *frameData)
		{
			node->Sample(animation, frame);
		}
	}
}

void AnimationPlayer::UpdateInternal(float deltaTime)
{
	// Iterate over each active animation state and update it.
	auto it = mActiveAnimationStates.begin();
	while(it != mActiveAnimationStates.end())
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
					node->Play(animState.animation);
				}
			}
			
			// Move on to the next frame.
			animState.currentFrame++;
			
			// This causes the animation to loop, but we don't really want that in GK3.
			//activeAnimation.currentFrame %= activeAnimation.animation->GetFrameCount();
			
			// Decrement timer, since we just simulated a single frame.
			animState.timer -= timePerFrame;
		}
		
		// If the animation has ended, remove it from the active animation states.
		if(animState.currentFrame >= animState.animation->GetFrameCount())
		{
			//std::cout << "Animation " << animState.animation->GetName() << " has ended." << std::endl;
			it = mActiveAnimationStates.erase(it);
			
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
