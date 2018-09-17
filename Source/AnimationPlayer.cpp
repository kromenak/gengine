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

void AnimationPlayer::Update(float deltaTime)
{
    // We need an animation to update.
    if(mAnimation != nullptr)
    {
        // Update current frame based on time that's passed.
        mAnimationTimer += deltaTime;
        float timePerFrame = 1.0f / mAnimation->GetFramesPerSecond();
        while(mAnimationTimer > timePerFrame)
        {
            // Execute actions for the current frame.
            std::vector<AnimationNode*>* frameData = mAnimation->GetFrame(mCurrentAnimationFrame);
            if(frameData != nullptr)
            {
                for(auto& node : *frameData)
                {
                    Play(node->mVertexAnimation);
                }
            }
            
            // Move on to the next frame.
            mCurrentAnimationFrame++;
            mCurrentAnimationFrame %= mAnimation->GetFrameCount();
            
            // Decrement frames passed counter.
            mAnimationTimer -= timePerFrame;
        }
    }
    
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
            meshes[i]->SetPositions((float*)sample.mVertexPositions.data());
            
            VertexAnimationTransformPose transformSample = mVertexAnimation->SampleTransformPose(mVertexAnimationTimer, framesPerSecond, i);
            meshes[i]->SetLocalTransformMatrix(transformSample.GetLocalTransformMatrix());
        }
    }
}

void AnimationPlayer::Play(Animation* animation)
{
    // Save animation.
    mAnimation = animation;
    
    // Start at the first frame.
    mCurrentAnimationFrame = 0;
    mAnimationTimer = 0.0f;
}

void AnimationPlayer::Play(VertexAnimation* vertexAnimation)
{
    // Save vertex animation.
    mVertexAnimation = vertexAnimation;
    
    // Start at beginning.s
    mVertexAnimationTimer = 0.0f;
}
