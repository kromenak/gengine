//
// GKObject.cpp
//
// Clark Kromenaker
//
#include "GKObject.h"

#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "VertexAnimationPlayer.h"

GKObject::GKObject() : GKObject(true)
{
	
}

GKObject::GKObject(bool defaultSetup)
{
	if(defaultSetup)
	{
		// Add mesh renderer.
		mMeshRenderer = AddComponent<MeshRenderer>();
		
		// Add vertex animation player.
		mVertexAnimationPlayer = AddComponent<VertexAnimationPlayer>();
		
		// Add GAS player.
		mGasPlayer = AddComponent<GasPlayer>();
	}
}

void GKObject::SetHeading(const Heading& heading)
{
	SetRotation(Quaternion(Vector3::UnitY, heading.ToRadians()));
}

Heading GKObject::GetHeading() const
{
	return Heading::FromQuaternion(GetRotation());
}

void GKObject::PlayAnimation(VertexAnimation* animation)
{
	PlayAnimation(animation, VertexAnimationPlayer::kDefaultFramesPerSecond);
}

void GKObject::PlayAnimation(VertexAnimation* animation, int framesPerSecond)
{
	mVertexAnimationPlayer->Play(animation, framesPerSecond);
}

void GKObject::SampleAnimation(VertexAnimation* animation, int frame)
{
	mVertexAnimationPlayer->Sample(animation, frame);
}
