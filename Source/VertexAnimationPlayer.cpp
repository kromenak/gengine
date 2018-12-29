//
// VertexAnimationPlayer.cpp
//
// Clark Kromenaker
//
#include "VertexAnimationPlayer.h"

#include <vector>

#include "Actor.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "VertexAnimation.h"

TYPE_DEF_CHILD(Component, VertexAnimationPlayer);

VertexAnimationPlayer::VertexAnimationPlayer(Actor* owner) : Component(owner)
{
	mMeshRenderer = owner->GetComponent<MeshRenderer>();
}

void VertexAnimationPlayer::Play(VertexAnimation* animation)
{
	mVertexAnimation = animation;
	mVertexAnimationTimer = 0.0f;
}

void VertexAnimationPlayer::Play(VertexAnimation* animation, int framesPerSecond)
{
	mFramesPerSecond = framesPerSecond;
	mVertexAnimation = animation;
	mVertexAnimationTimer = 0.0f;
}

void VertexAnimationPlayer::Sample(VertexAnimation* animation, int frame)
{
	//TODO: We kind of convert from frames to time, then back to frames. Why not just stay in frames then?
	float time = (float)frame * mFramesPerSecond;
	TakeSample(animation, time);
}

void VertexAnimationPlayer::UpdateInternal(float deltaTime)
{
	// Need a vertex animation to update.
	if(mVertexAnimation == nullptr) { return; }
	
	// Increment animation timer.
	mVertexAnimationTimer += deltaTime;
	
	// Sample animation at current timer value.
	TakeSample(mVertexAnimation, mVertexAnimationTimer);
	
	// If at the end of the animation, clear animation.
	// GK3 doesn't really have the concept of a "looping" animation. Looping is handled by higher-level control scripts.
	if(mVertexAnimationTimer >= mVertexAnimation->GetDuration(mFramesPerSecond))
	{
		mVertexAnimation = nullptr;
	}
}

void VertexAnimationPlayer::TakeSample(VertexAnimation* animation, float time)
{
	if(animation == nullptr) { return; }
	
	// Iterate through each mesh and sample it in the vertex animation.
	// We need to sample both vertex poses and transform poses to get the right result.
	std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
	for(int i = 0; i < meshes.size(); i++)
	{
		const std::vector<Submesh*>& submeshes = meshes[i]->GetSubmeshes();
		for(int j = 0; j < submeshes.size(); j++)
		{
			VertexAnimationVertexPose sample = animation->SampleVertexPose(time, mFramesPerSecond, i, j);
			if(sample.mFrameNumber >= 0)
			{
				submeshes[j]->CopyPositions((float*)sample.mVertexPositions.data());
			}
		}
		
		VertexAnimationTransformPose transformSample = animation->SampleTransformPose(time, mFramesPerSecond, i);
		if(transformSample.mFrameNumber >= 0)
		{
			meshes[i]->SetLocalTransformMatrix(transformSample.GetLocalTransformMatrix());
		}
	}
}
