//
// VertexAnimator.cpp
//
// Clark Kromenaker
//
#include "VertexAnimator.h"

#include <vector>

#include "Actor.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "VertexAnimation.h"

TYPE_DEF_CHILD(Component, VertexAnimator);

VertexAnimator::VertexAnimator(Actor* owner) : Component(owner)
{
	mMeshRenderer = owner->GetComponent<MeshRenderer>();
}

void VertexAnimator::Start(const VertexAnimParams& params)
{
    // If we're interrupting some other anim, stop it (fires stop callback).
    Stop();
    
    // Save parameters.
    mVertexAnimation = params.vertexAnimation;
    mFramesPerSecond = params.framesPerSecond;
    mAnimationTimer = params.startTime;
    mStopCallback = params.stopCallback;
    mFromAutoscript = params.fromAutoScript;
    
    // Sample animation immediately so mesh's positions/rotations are updated.
    TakeSample(mVertexAnimation, mAnimationTimer);
}

void VertexAnimator::Stop(VertexAnimation* anim)
{
	// Stop if animation matches playing one OR null was passed in.
	if(mVertexAnimation != nullptr && (mVertexAnimation == anim || anim == nullptr))
	{
		// Fire stop callback if an animation was in progress.
		if(mStopCallback != nullptr)
		{
			mStopCallback();
		}
		
		// Reset state data.
		mVertexAnimation = nullptr;
		mStopCallback = nullptr;
	}
}

void VertexAnimator::Sample(VertexAnimation* animation, int frame)
{
	if(animation != nullptr)
	{
		TakeSample(animation, frame);
	}
}

void VertexAnimator::OnUpdate(float deltaTime)
{
	// Need a vertex animation to update.
	if(mVertexAnimation != nullptr)
	{
		// Increment animation timer.
		mAnimationTimer += deltaTime;
		
		// Sample animation at current timer value, clamping to anim duration.
		float animDuration = mVertexAnimation->GetDuration(mFramesPerSecond);
		TakeSample(mVertexAnimation, Math::Clamp(mAnimationTimer, 0.0f, animDuration));
		
		// If at the end of the animation, clear animation.
		// GK3 doesn't really have the concept of a "looping" animation. Looping is handled by higher-level control scripts.
		if(mAnimationTimer >= animDuration)
		{
			Stop(mVertexAnimation);
		}
	}
}

void VertexAnimator::TakeSample(VertexAnimation* animation, int frame)
{
    // Iterate through each mesh and sample it in the vertex animation.
    // We need to sample both vertex poses and transform poses to get the right result.
    const std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
    for(int i = 0; i < meshes.size(); i++)
    {
        const std::vector<Submesh*>& submeshes = meshes[i]->GetSubmeshes();
        for(int j = 0; j < submeshes.size(); j++)
        {
            VertexAnimationVertexPose sample = animation->SampleVertexPose(frame, i, j);
            if(sample.mFrameNumber >= 0)
            {
                submeshes[j]->SetPositions(reinterpret_cast<float*>(sample.mVertexPositions.data()), true);
            }
        }
        
        VertexAnimationTransformPose transformSample = animation->SampleTransformPose(frame, i);
        if(transformSample.mFrameNumber >= 0)
        {
            meshes[i]->SetMeshToLocalMatrix(transformSample.mMeshToLocalMatrix);
        }
    }
}

void VertexAnimator::TakeSample(VertexAnimation* animation, float time)
{
	// Iterate through each mesh and sample it in the vertex animation.
	// We need to sample both vertex poses and transform poses to get the right result.
	const std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
	for(int i = 0; i < meshes.size(); i++)
	{
		const std::vector<Submesh*>& submeshes = meshes[i]->GetSubmeshes();
		for(int j = 0; j < submeshes.size(); j++)
		{
			VertexAnimationVertexPose sample = animation->SampleVertexPose(time, mFramesPerSecond, i, j);
			if(sample.mFrameNumber >= 0)
			{
                submeshes[j]->SetPositions(reinterpret_cast<float*>(sample.mVertexPositions.data()), true);
			}
		}
		
		VertexAnimationTransformPose transformSample = animation->SampleTransformPose(time, mFramesPerSecond, i);
		if(transformSample.mFrameNumber >= 0)
		{
            meshes[i]->SetMeshToLocalMatrix(transformSample.mMeshToLocalMatrix);
		}
	}
}
