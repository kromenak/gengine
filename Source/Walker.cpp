//
// Walker.cpp
//
// Clark Kromenaker
//
#include "Walker.h"

#include "Actor.h"
#include "CharacterManager.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Scene.h"
#include "Vector3.h"

TYPE_DEF_CHILD(Component, Walker);

Walker::Walker(Actor* owner) : Component(owner)
{
	/*
	GKActor* actor = mOwner->GetComponent<GKActor>();
	if(actor == nullptr)
	{
		std::cout << "Walker does not have a GKActor! It won't be very helpful!" << std::endl;
		return;
	}
	
	CharacterConfig& actorConfig = Services::Get<CharacterManager>()->GetCharacterConfig(actor->GetIdentifier());
	mWalkStartAnim = actorConfig.walkStartAnim;
	mWalkLoopAnim = actorConfig.walkLoopAnim;
	mWalkStopAnim = actorConfig.walkStopAnim;
	*/
}

void Walker::UpdateInternal(float deltaTime)
{
	// Stay attached to the floor.
	Vector3 position = mOwner->GetPosition();
	position.SetY(GEngine::inst->GetScene()->GetFloorY(position));
	mOwner->SetPosition(position);
}
