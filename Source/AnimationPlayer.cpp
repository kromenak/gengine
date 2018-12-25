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
	mActiveAnimations.emplace_back(animation, 0, 0.0f);
	
	/*
    // Save animation.
    mAnimation = animation;
    
    // Start at the first frame.
    mCurrentAnimationFrame = 0;
    mAnimationTimer = 0.0f;
	*/
}

void AnimationPlayer::Play(VertexAnimation* vertexAnimation)
{
	/*
    // Save vertex animation.
    mVertexAnimation = vertexAnimation;
    
    // Start at beginning.s
    mVertexAnimationTimer = 0.0f;
	*/
}

void AnimationPlayer::Sample(Animation* animation, int frame)
{
	/*
	if(animation == nullptr) { return; }
	
	std::vector<AnimNode*>* frameData = animation->GetFrame(frame);
	if(frameData == nullptr) { return; }
	
	if(frameData->size() > 0)
	{
		AnimNode* node = (*frameData)[0];
		if(node->mVertexAnimation == nullptr) { return; }
		
		// Iterate through each mesh and sample it in the vertex animation.
		// We need to sample both vertex poses and transform poses to get the right result.
		std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
		for(int i = 0; i < meshes.size(); i++)
		{
			VertexAnimationVertexPose sample = node->mVertexAnimation->SampleVertexPose(mVertexAnimationTimer, 15, i);
			if(sample.mFrameNumber >= 0)
			{
				meshes[i]->SetPositions((float*)sample.mVertexPositions.data());
			}
			
			VertexAnimationTransformPose transformSample = node->mVertexAnimation->SampleTransformPose(mVertexAnimationTimer, 15, i);
			if(transformSample.mFrameNumber >= 0)
			{
				meshes[i]->SetLocalTransformMatrix(transformSample.GetLocalTransformMatrix());
			}
		}
	}
	*/
}

void AnimationPlayer::UpdateInternal(float deltaTime)
{
	auto it = mActiveAnimations.begin();
	while(it != mActiveAnimations.end())
	{
		AnimationState& activeAnimation = *it;
		
		activeAnimation.timer += deltaTime;
		float timePerFrame = 1.0f / activeAnimation.animation->GetFramesPerSecond();
		while(activeAnimation.timer > timePerFrame)
		{
			// Execute actions for the current frame.
			std::vector<AnimNode*>* frameData = activeAnimation.animation->GetFrame(activeAnimation.currentFrame);
			if(frameData != nullptr)
			{
				for(auto& node : *frameData)
				{
					node->Play();
					//Play(node->mVertexAnimation);
				}
			}
			
			// Move on to the next frame.
			activeAnimation.currentFrame++;
			//activeAnimation.currentFrame %= activeAnimation.animation->GetFrameCount();
			
			// Decrement frames passed counter.
			activeAnimation.timer -= timePerFrame;
		}
		
		if(activeAnimation.currentFrame >= activeAnimation.animation->GetFrameCount())
		{
			it = mActiveAnimations.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	/*
	// We need an animation to update.
	if(mAnimation == nullptr) { return; }
	
	// Update current frame based on time that's passed.
	mAnimationTimer += deltaTime;
	float timePerFrame = 1.0f / mAnimation->GetFramesPerSecond();
	while(mAnimationTimer > timePerFrame)
	{
		// Execute actions for the current frame.
		std::vector<AnimNode*>* frameData = mAnimation->GetFrame(mCurrentAnimationFrame);
		if(frameData != nullptr)
		{
			for(auto& node : *frameData)
			{
				node->Play();
				//Play(node->mVertexAnimation);
			}
		}
		
		// Move on to the next frame.
		mCurrentAnimationFrame++;
		mCurrentAnimationFrame %= mAnimation->GetFrameCount();
		
		// Decrement frames passed counter.
		mAnimationTimer -= timePerFrame;
	}
	*/
	 
	/*
	// Update the vertex animation, if any.
	if(mVertexAnimation != nullptr)
	{
		// Determine FPS for sampling animation.
		// This is taken from the animation - the vertex animation itself doesn't store this data.
		int framesPerSecond = 15;
		if(mAnimation != nullptr)
		{
			framesPerSecond = mAnimation->GetFramesPerSecond();
		}
		
		// Increment animation timer.
		mVertexAnimationTimer += deltaTime;
		
		// Iterate through each mesh and sample it in the vertex animation.
		// We need to sample both vertex poses and transform poses to get the right result.
		std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
		for(int i = 0; i < meshes.size(); i++)
		{
			VertexAnimationVertexPose sample = mVertexAnimation->SampleVertexPose(mVertexAnimationTimer, framesPerSecond, i);
			if(sample.mFrameNumber >= 0)
			{
				meshes[i]->SetPositions((float*)sample.mVertexPositions.data());
			}
			
			VertexAnimationTransformPose transformSample = mVertexAnimation->SampleTransformPose(mVertexAnimationTimer, framesPerSecond, i);
			if(transformSample.mFrameNumber >= 0)
			{
				meshes[i]->SetLocalTransformMatrix(transformSample.GetLocalTransformMatrix());
			}
		}
	}
	*/
}
