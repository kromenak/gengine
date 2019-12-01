//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Services.h"
#include "VertexAnimationPlayer.h"
#include "Walker.h"

GKActor::GKActor(const std::string& identifier) : GKObject(true),
	mIdentifier(identifier)
{
	// For some reason, characters (Gabe, Grace, etc) face backwards by default.
	// To remedy this, I'm sticking the mesh render in a child object, rotated 180 degrees.
	// Props don't seem to have this problem. What gives!?
	//Actor* meshActor = new Actor();
	//Transform* meshActorTransform = meshActor->GetComponent<Transform>();
	//meshActorTransform->SetParent(GetComponent<Transform>());
	//meshActorTransform->SetPosition(Vector3::Zero);
	//meshActorTransform->SetRotation(Quaternion(Vector3::UnitY, Math::kPi));
	//mMeshRenderer = meshActor->AddComponent<MeshRenderer>();
	
	// Get config for this character.
	CharacterConfig& config = Services::Get<CharacterManager>()->GetCharacterConfig(mIdentifier);
	
	// Add walker and configure it.
	Actor* walkerActor = new Actor();
	mWalker = walkerActor->AddComponent<Walker>();
	mWalker->SetCharacterConfig(config);
	mWalker->SetWalkActor(this);
	
    // Create animation player on the same objwdect as the mesh renderer.
    //mVertexAnimationPlayer = mMeshRenderer->GetOwner()->AddComponent<VertexAnimationPlayer>();
    
    // GasPlayer is used to play gas script files.
    // Actor also needs a way to play animations, and the GasPlayer needs to know about it.
    //mGasPlayer = AddComponent<GasPlayer>();
	
	// Create and configure face controller.
	mFaceController = AddComponent<FaceController>();
	mFaceController->SetCharacterConfig(config);
}

void GKActor::StartFidget(FidgetType type)
{
    // Save type.
    mActiveFidget = type;
    
    // Set appropriate gas to play.
    switch(mActiveFidget)
    {
	case FidgetType::None:
	case FidgetType::Custom: // Can only be started via custom GAS version of this function (for now).
		mGasPlayer->SetGas(nullptr);
		break;
		
	case FidgetType::Idle:
		mGasPlayer->SetGas(mIdleGas);
		break;
		
	case FidgetType::Talk:
		mGasPlayer->SetGas(mTalkGas);
		break;
		
	case FidgetType::Listen:
		mGasPlayer->SetGas(mListenGas);
		break;
    }
}

void GKActor::StartCustomFidget(GAS* gas)
{
	mActiveFidget = FidgetType::Custom;
	mGasPlayer->SetGas(gas);
}

void GKActor::OnUpdate(float deltaTime)
{
	// Pause any fidgets while walker is going.
	if(mGasPlayer != nullptr && mWalker != nullptr)
	{
		mGasPlayer->SetPaused(mWalker->IsWalking());
	}
}
