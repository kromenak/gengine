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
    mCurrentParams = params;
    mAnimationTimer = params.startTime;
    
    // Sample animation immediately so mesh's positions/rotations are updated.
    TakeSample(mCurrentParams.vertexAnimation, mAnimationTimer);

    // The disabled timer is only useful to track how long this component was disabled so an animation can catch up.
    // When a new animation starts, we should reset this so it times from the beginning of the new animation.
    mDisabledTimer.Reset();
}

void VertexAnimator::Stop(VertexAnimation* anim)
{
	// Stop if animation matches playing one OR null was passed in.
	if(mCurrentParams.vertexAnimation != nullptr && (mCurrentParams.vertexAnimation == anim || anim == nullptr))
	{
		// Fire stop callback if an animation was in progress.
		if(mCurrentParams.stopCallback != nullptr)
		{
            mCurrentParams.stopCallback();
		}
		
		// Reset state data.
        mCurrentParams.vertexAnimation = nullptr;
        mCurrentParams.stopCallback = nullptr;
	}
}

void VertexAnimator::Sample(VertexAnimation* animation, int frame)
{
	if(animation != nullptr)
	{
		TakeSample(animation, frame);
	}
}

void VertexAnimator::OnEnable()
{
    // When we are enabled, perform a BIG update to correspond with the disabled period.
    // This lets the animation "catch up" (as though the playing animation was playing the entire time the object was disabled).
    OnUpdate(mDisabledTimer.GetSeconds());
}

void VertexAnimator::OnDisable()
{
    // On disable, reset our disabled timer.
    mDisabledTimer.Reset();
}

void VertexAnimator::OnUpdate(float deltaTime)
{
	// Need a vertex animation to update.
	if(mCurrentParams.vertexAnimation != nullptr)
	{
		// Increment animation timer.
		mAnimationTimer += deltaTime;
		
		// Sample animation at current timer value, clamping to anim duration.
		float animDuration = mCurrentParams.vertexAnimation->GetDuration(mCurrentParams.framesPerSecond);
		TakeSample(mCurrentParams.vertexAnimation, Math::Clamp(mAnimationTimer, 0.0f, animDuration));
		
		// If at the end of the animation, clear animation.
		// GK3 doesn't really have the concept of a "looping" animation. Looping is handled by higher-level control scripts.
		if(mAnimationTimer >= animDuration)
		{
			Stop(mCurrentParams.vertexAnimation);
		}

        // Position relative to parent.
        if(mCurrentParams.parent != nullptr)
        {
            GetOwner()->GetTransform()->SetPosition(mCurrentParams.parent->GetPosition());
            GetOwner()->GetTransform()->SetRotation(mCurrentParams.parent->GetRotation());
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
            if(sample.frameNumber >= 0)
            {
                submeshes[j]->SetPositions(reinterpret_cast<float*>(sample.vertexPositions.data()));
            }
        }
        
        VertexAnimationTransformPose transformSample = animation->SampleTransformPose(frame, i);
        if(transformSample.frameNumber >= 0)
        {
            meshes[i]->SetMeshToLocalMatrix(transformSample.meshToLocalMatrix);
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
			VertexAnimationVertexPose sample = animation->SampleVertexPose(time, mCurrentParams.framesPerSecond, i, j);
			if(sample.frameNumber >= 0)
			{
                submeshes[j]->SetPositions(reinterpret_cast<float*>(sample.vertexPositions.data()));
			}
		}
		
		VertexAnimationTransformPose transformSample = animation->SampleTransformPose(time, mCurrentParams.framesPerSecond, i);
		if(transformSample.frameNumber >= 0)
		{
            meshes[i]->SetMeshToLocalMatrix(transformSample.meshToLocalMatrix);
		}
	}
}
