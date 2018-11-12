//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include "AnimationPlayer.h"
#include "Debug.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Scene.h"

GKActor::GKActor() : Actor()
{
    // Usually, we'll have a mesh to display.
    mMeshRenderer = AddComponent<MeshRenderer>();
    
    // Create animation player.
    AnimationPlayer* animationPlayer = AddComponent<AnimationPlayer>();
    animationPlayer->SetMeshRenderer(mMeshRenderer);
    
    // GasPlayer is used to play gas script files.
    // Actor also needs a way to play animations, and the GasPlayer needs to know about it.
    mGasPlayer = AddComponent<GasPlayer>();
    mGasPlayer->SetAnimationPlayer(animationPlayer);
	
	Debug::DrawLine(Vector3::UnitX * 50, Vector3::UnitX * 100, Color32::Magenta, 10.0f);
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

void GKActor::UpdateInternal(float deltaTime)
{
	// Stay attached to the floor, usually.
	Vector3 position = GetPosition();
	position.SetY(GEngine::inst->GetScene()->GetFloorY(position));
	SetPosition(position);
}
