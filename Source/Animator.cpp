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

void Animator::Start(Animation* animation, bool allowMove, bool fromGas, std::function<void()> finishCallback)
{
	if(animation == nullptr) { return; }
	
	// Create anim state for animation with appropriate "allow move" value.
	mActiveAnimations.emplace_back(animation, finishCallback);
	mActiveAnimations.back().allowMove = allowMove;
	mActiveAnimations.back().fromGas = fromGas;
	
	// Immediately execute frame 0 of the animation.
	// Frames execute at the BEGINNING of the time slice for that frame, so frame 0 executes at t=0.
	ExecuteFrame(mActiveAnimations.back(), 0);
}

void Animator::Loop(Animation* animation)
{
	if(animation == nullptr) { return; }
	
	// Can start anim per usual, but just set loop flag after creation.
	Start(animation, false, false, nullptr);
	mActiveAnimations.back().loop = true;
}

void Animator::Stop(Animation* animation)
{
	if(animation == nullptr) { return; }
	
	// Remove all anim states that are using the passed-in animation.
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
	
	// "remove_if" returns iterator to new ending (all elements to be erased are after it.
	// So...do the erase!
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
		// Increment animation timer.
		AnimationState& animState = *it;
		animState.timer += deltaTime;
		
		/*
		 Say we have a 6-frame animation:
		 0----1----2----3----4----5-----
		 
		 Frame 0 executes immediately when the animation starts.
		 After that, each frame executes after X seconds have passed.
		 
		 One issue that arises with "looped" animations particularly is that
		 the first and last frames are the same pose (0/5 in above example). And in that
		 case, you don't want that extra "time" after frame 5 to occur, or the
		 looped animation stutters when it loops.
		 That's why we use "-1" to decide when to loop/finish the anim.
		 TODO: Does that cause problems with non-looping anims? Need to see!
		 */
		
		// Based on how much time has passed, we may need to increment multiple frames of animation in one update loop.
		// For example, if timer is 0.3, and timePerFrame is 0.1, we need to update 3 times.
		float timePerFrame = animState.animation->GetFrameDuration();
		while(animState.timer >= timePerFrame)
		{
			// WE ARE EXECUTING A FRAME!
			
			// Decrement timer by amount for one frame.
			// "timer" now contains how much time we are ahead of the current frame.
			animState.timer -= timePerFrame;
			
			// Increment the frame.
			// Frame 0 happens immediately on anim start. Each executed frame is then one more.
			animState.currentFrame++;
			
			// If looping, wrap around the current frame when we reach the end!
			// Note the "-1" because first and last frames are the same for a looping anim!
			if(animState.loop)
			{
				animState.currentFrame %= animState.animation->GetFrameCount() - 1;
			}
			
			// Execute any actions/anim nodes on the current frame.
			ExecuteFrame(animState, animState.currentFrame);
		}
		
		// If the animation has ended, remove it from the active animation states.
		if(animState.currentFrame >= animState.animation->GetFrameCount() - 1)
		{
			// Do the finish callback!
			if(animState.finishCallback)
			{
				animState.finishCallback();
			}

			// Erase the anim state.
			it = mActiveAnimations.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Animator::ExecuteFrame(AnimationState& animState, int frameNumber)
{
	std::vector<AnimNode*>* animNodes = animState.animation->GetFrame(frameNumber);
	if(animNodes != nullptr)
	{
		for(auto& node : *animNodes)
		{
			node->Play(&animState);
		}
	}
}
