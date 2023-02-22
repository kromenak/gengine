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
        ExecuteFrame(mActiveAnimations.size() - 1, i);
    }
}

void Animator::Stop(Animation* animation)
{
	if(animation == nullptr) { return; }

    // Prematurely stop any active anim states matching this animation.
    for(auto& animState : mActiveAnimations)
    {
        if(!animState.done && animState.params.animation == animation)
        {
            animState.Stop();
        }
    }
}

void Animator::StopAll()
{
    for(auto& animState : mActiveAnimations)
    {
        animState.Stop();
    }
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
			node->Sample(frame);
		}
	}
}

void Animator::Sample(Animation* animation, int frame, const std::string& modelName)
{
    if(animation == nullptr) { return; }

    // Similar to above, but ONLY sample nodes that are relevant to this model.
    std::vector<AnimNode*>* frameData = animation->GetFrame(frame);
    if(frameData != nullptr)
    {
        for(auto& node : *frameData)
        {
            if(node->AppliesToModel(modelName))
            {
                node->Sample(frame);
            }
        }
    }
}

void Animator::OnUpdate(float deltaTime)
{
	// Iterate over each active animation state and update it.
    int size = mActiveAnimations.size();
    for(int i = 0; i < size; ++i)
    {
        // Skip any entries that are done.
        if(mActiveAnimations[i].done) { continue; }

        // Increment animation timer.
        mActiveAnimations[i].timer += deltaTime;
		
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
		float timePerFrame = mActiveAnimations[i].params.animation->GetFrameDuration();
		while(mActiveAnimations[i].timer >= timePerFrame)
		{
			// WE ARE EXECUTING A FRAME!
			
			// Decrement timer by amount for one frame.
			// "timer" now contains how much time we are ahead of the current frame.
            mActiveAnimations[i].timer -= timePerFrame;
			
			// Increment the frame.
			// Frame 0 happens immediately on anim start. Each executed frame is then one more.
            mActiveAnimations[i].currentFrame++;
			
			// If looping, wrap around the current frame when we reach the end!
			// Note the "-1" because first and last frames are the same for a looping anim!
			if(mActiveAnimations[i].params.loop)
			{
                mActiveAnimations[i].currentFrame %= mActiveAnimations[i].params.animation->GetFrameCount() - 1;
			}

            // Break out of loop if the animation has ended.
            // In extreme cases, the timer could be large enough to cover frames that don't exist - just ignore that!
            if(mActiveAnimations[i].currentFrame >= mActiveAnimations[i].params.animation->GetFrameCount())
            {
                break;
            }
			
			// Execute any actions/anim nodes on the current frame.
			ExecuteFrame(i, mActiveAnimations[i].currentFrame);
		}

		// If the animation has finished, mark it as done.
        if(mActiveAnimations[i].currentFrame >= mActiveAnimations[i].params.animation->GetFrameCount() - 1)
        {
            mActiveAnimations[i].done = true;
        }
    }

    // Process any finish callbacks.
    // FYI: This can lead to recursive sorts of things where new active animations are added or scene is changed (destroying the Animator!).
    // So, gotta be a bit careful here...
    size = mActiveAnimations.size();
    for(int i = 0; i < size; ++i)
    {
        if(mActiveAnimations[i].done && mActiveAnimations[i].params.finishCallback != nullptr)
        {
            mActiveAnimations[i].params.finishCallback();
        }
    }

    // Remove animations that are done.
    for(int i = size - 1; i >= 0; --i)
    {
        if(mActiveAnimations[i].done)
        {
            mActiveAnimations.erase(mActiveAnimations.begin() + i);
        }
    }
}

void Animator::ExecuteFrame(int animIndex, int frameNumber)
{
    // Save executing frame #.
    mActiveAnimations[animIndex].executingFrame = frameNumber;
    
    // Get all anim nodes that begin on this frame and start them.
	std::vector<AnimNode*>* animNodes = mActiveAnimations[animIndex].params.animation->GetFrame(frameNumber);
	if(animNodes != nullptr)
	{
		for(auto& node : *animNodes)
		{
            // If current frame != executing frame, we are "fast forwarding" - executing this frame to catch up.
            // Most nodes don't support this (mostly just vertex anim nodes). So, skip unsupported nodes during catchup.
            if(mActiveAnimations[animIndex].currentFrame != mActiveAnimations[animIndex].executingFrame && !node->PlayDuringCatchup())
            {
                continue;
            }
            
            // Play the node!
			node->Play(&mActiveAnimations[animIndex]);
		}
	}
}

void AnimationState::Stop()
{
    // If stopping an animation, be sure to also stop any running vertex animations.
    if(!done)
    {
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

    // Done if stopped.
    done = true;
}
