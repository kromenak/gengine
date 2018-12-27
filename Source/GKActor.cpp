//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include "Debug.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "VertexAnimationPlayer.h"

GKActor::GKActor(bool forCharacter) : Actor()
{
    // Usually, we'll have a mesh to render.
	// For some reason, characters (Gabe, Grace, etc) face backwards by default.
	// To remedy this, I'm sticking the mesh render in a child object, rotated 180 degrees.
	// Props don't seem to have this problem. What gives!?
	if(forCharacter)
	{
		Actor* meshParent = new Actor();
		Transform* meshParentTransform = meshParent->GetComponent<Transform>();
		meshParentTransform->SetParent(GetComponent<Transform>());
		meshParentTransform->SetPosition(Vector3::Zero);
		meshParentTransform->SetRotation(Quaternion(Vector3::UnitY, Math::kPi));
		mMeshRenderer = meshParent->AddComponent<MeshRenderer>();
	}
	else
	{
    	mMeshRenderer = AddComponent<MeshRenderer>();
	}
    
    // Create animation player on the same object as the mesh renderer.
    mAnimationPlayer = mMeshRenderer->GetOwner()->AddComponent<VertexAnimationPlayer>();
    
    // GasPlayer is used to play gas script files.
    // Actor also needs a way to play animations, and the GasPlayer needs to know about it.
    mGasPlayer = AddComponent<GasPlayer>();
}

void GKActor::SetState(GKActor::State state)
{
    // Save state.
    mState = state;
    
    // Set appropriate Gas to play.
    switch(mState)
    {
	case State::Idle:
		mGasPlayer->SetGas(mIdleGas);
		break;
		
	case State::Talk:
		mGasPlayer->SetGas(mTalkGas);
		break;
		
	case State::Listen:
		mGasPlayer->SetGas(mListenGas);
		break;
    }
}

void GKActor::PlayAnimation(VertexAnimation* animation)
{
	mAnimationPlayer->Play(animation);
}

void GKActor::PlayAnimation(VertexAnimation* animation, int framesPerSecond)
{
	mAnimationPlayer->Play(animation, framesPerSecond);
}

void GKActor::UpdateInternal(float deltaTime)
{
	// Stay attached to the floor, usually.
	Vector3 position = GetPosition();
	position.SetY(GEngine::inst->GetScene()->GetFloorY(position));
	SetPosition(position);
}
