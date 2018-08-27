//
// AnimationPlayer.cpp
//
// Clark Kromenaker
//
#include "AnimationPlayer.h"
#include "Animation.h"
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
        int framesPassed = deltaTime * mAnimation->GetFramesPerSecond();
        while(framesPassed > 0)
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
            framesPassed--;
        }
    }
    
    // Update the vertex animation, if any.
    if(mVertexAnimation != nullptr)
    {
        mVertexAnimationTimer += deltaTime;
        
        std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
        
        //std::vector<Vector3> sample = mVertexAnimation->TakeSample(mVertexAnimationTimer, 4);
        //float* fPtr = (float*)sample.data();
        //meshes[4]->SetPositions(fPtr);
        
        for(int i = 0; i < meshes.size(); i++)
        {
            VertexAnimationMeshPose sample = mVertexAnimation->TakeSample(mVertexAnimationTimer, i);
            float* fPtr = (float*)sample.mVertexPositions.data();
            meshes[i]->SetPositions(fPtr);
            meshes[i]->SetLocalTransformMatrix(sample.GetLocalTransformMatrix());
        }
    }
}

void AnimationPlayer::Play(Animation* animation)
{
    mAnimation = animation;
    
    // Start at the first frame.
    mCurrentAnimationFrame = 0;
}

void AnimationPlayer::Play(VertexAnimation* vertexAnimation)
{
    mVertexAnimation = vertexAnimation;
    mVertexAnimationTimer = 0.0f;
}
