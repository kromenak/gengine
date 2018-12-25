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

void VertexAnimationPlayer::UpdateInternal(float deltaTime)
{
	// Need a vertex animation to update.
	if(mVertexAnimation == nullptr) { return; }
	
	// Increment animation timer.
	mVertexAnimationTimer += deltaTime;
	
	// Iterate through each mesh and sample it in the vertex animation.
	// We need to sample both vertex poses and transform poses to get the right result.
	std::vector<Mesh*> meshes = mMeshRenderer->GetMeshes();
	for(int i = 0; i < meshes.size(); i++)
	{
		VertexAnimationVertexPose sample = mVertexAnimation->SampleVertexPose(mVertexAnimationTimer, mFramesPerSecond, i);
		if(sample.mFrameNumber >= 0)
		{
			meshes[i]->SetPositions((float*)sample.mVertexPositions.data());
		}
		
		VertexAnimationTransformPose transformSample = mVertexAnimation->SampleTransformPose(mVertexAnimationTimer, mFramesPerSecond, i);
		if(transformSample.mFrameNumber >= 0)
		{
			meshes[i]->SetLocalTransformMatrix(transformSample.GetLocalTransformMatrix());
		}
	}
}
