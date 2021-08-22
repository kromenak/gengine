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

void Animator::Start(Animation* animation, std::function<void()> finishCallback)
{
    AnimParams params;
    params.animation = animation;
    params.finishCallback = finishCallback;
    Start(params);
}

void Animator::Start(const AnimParams& animParams)
{
    if(animParams.animation == nullptr)
    {
        if(animParams.finishCallback != nullptr)
        {
            animParams.finishCallback();
        }
        return;
    }
    
    // Create anim state for animation with appropriate "allow move" value.
    mActiveAnimations.emplace_back();
    mActiveAnimations.back().params = animParams;

    // Immediately execute start frame of the animation.
    // Frames execute at the BEGINNING of the time slice for that frame, so frame 0 executes at t=0.
    for(int i = 0; i <= animParams.startFrame; ++i)
    {
        ExecuteFrame(mActiveAnimations.back(), i);
    }
}

void Animator::Stop(Animation* animation)
{
	if(animation == nullptr) { return; }
	
	// Remove all anim states that are using the passed-in animation.
	auto newEndIt = std::remove_if(mActiveAnimations.begin(), mActiveAnimations.end(), [animation](AnimationState& animState) -> bool {
		if(animState.params.animation == animation)
		{
            animState.Stop();
			return true;
		}
		return false;
	});
	
	// "remove_if" returns iterator to new ending (all elements to be erased are after it).
	// So...do the erase!
	mActiveAnimations.erase(newEndIt, mActiveAnimations.end());
}

void Animator::StopAll()
{
    for(auto& animState : mActiveAnimations)
    {
        animState.Stop();
    }
    mActiveAnimations.clear();
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
		 */
		
		// Based on how much time has passed, we may need to increment multiple frames of animation in one update loop.
		// For example, if timer is 0.3, and timePerFrame is 0.1, we need to update 3 times.
		float timePerFrame = animState.params.animation->GetFrameDuration();
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
			if(animState.params.loop)
			{
				animState.currentFrame %= animState.params.animation->GetFrameCount() - 1;
			}

            // Break out of loop if the animation has ended.
            // In extreme cases, the timer could be large enough to cover frames that don't exist - just ignore that!
            if(animState.currentFrame >= animState.params.animation->GetFrameCount())
            {
                break;
            }
			
			// Execute any actions/anim nodes on the current frame.
			ExecuteFrame(animState, animState.currentFrame);
		}
		
		// If the animation has ended, remove it from the active animation states.
		if(animState.currentFrame >= animState.params.animation->GetFrameCount() - 1)
		{
			// Do the finish callback!
			if(animState.params.finishCallback != nullptr)
			{
				animState.params.finishCallback();
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
    // Save executing frame #.
    animState.executingFrame = frameNumber;
    
    // Get all anim nodes that begin on this frame and start them.
	std::vector<AnimNode*>* animNodes = animState.params.animation->GetFrame(frameNumber);
	if(animNodes != nullptr)
	{
		for(auto& node : *animNodes)
		{
            // If current frame != executing frame, we are "fast forwarding" - executing this frame to catch up.
            // Most nodes don't support this (mostly just vertex anim nodes). So, skip unsupported nodes during catchup.
            if(animState.currentFrame != animState.executingFrame && !node->PlayDuringCatchup())
            {
                continue;
            }
            
            // Play the node!
			node->Play(&animState);
		}
	}
}

void AnimationState::Stop()
{
    // If stopping an animation, be sure to also stop any running vertex animations.
    if(params.animation != nullptr)
    {
        auto& vertexAnims = params.animation->GetVertexAnimNodes();
        for(auto& vertexAnim : vertexAnims)
        {
            if(vertexAnim->frameNumber <= currentFrame)
            {
                vertexAnim->Stop();
            }
        }
    }
}
