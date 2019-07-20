//
// Walker.cpp
//
// Clark Kromenaker
//
#include "Walker.h"

#include "Actor.h"
#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Scene.h"
#include "Vector3.h"

TYPE_DEF_CHILD(Component, Walker);

Walker::Walker(Actor* owner) : Component(owner)
{
	
}

void Walker::SetCharacterConfig(const CharacterConfig& characterConfig)
{
	mWalkStartAnim = characterConfig.walkStartAnim;
	mWalkLoopAnim = characterConfig.walkLoopAnim;
	mWalkStopAnim = characterConfig.walkStopAnim;
}

void Walker::UpdateInternal(float deltaTime)
{
	// Get position to modify.
	Vector3 position = mOwner->GetPosition();
	
	// If we have a path, follow it.
	if(mPath.size() > 0)
	{
		Vector3 toNext = mPath[0] - position;
		if(toNext.GetLengthSq() < 1.0f)
		{
			mPath.erase(mPath.begin());
			if(mPath.size() <= 0)
			{
				StopWalk();
			}
		}
		
		Vector3 dir = toNext;
		dir.Normalize();
		position += dir * 25.0f * deltaTime;
	}
	
	// Stay attached to the floor.
	Scene* scene = GEngine::inst->GetScene();
	if(scene != nullptr)
	{
		position.SetY(scene->GetFloorY(position));
	}
	
	// Set updated position.
	mOwner->SetPosition(position);
}

void Walker::StartWalk()
{
	if(mState == State::Idle)
	{
		mState = State::Start;
		GEngine::inst->GetScene()->GetAnimationPlayer()->Play(mWalkStartAnim, std::bind(&Walker::ContinueWalk, this));
	}
}

void Walker::ContinueWalk()
{
	mState = State::Loop;
	GEngine::inst->GetScene()->GetAnimationPlayer()->Play(mWalkLoopAnim, std::bind(&Walker::ContinueWalk, this));
}

void Walker::StopWalk()
{
	if(mState != State::End)
	{
		GEngine::inst->GetScene()->GetAnimationPlayer()->Stop(mWalkStartAnim);
		GEngine::inst->GetScene()->GetAnimationPlayer()->Stop(mWalkLoopAnim);
		
		mState = State::End;
		GEngine::inst->GetScene()->GetAnimationPlayer()->Play(mWalkStopAnim, [this]() -> void {
			this->mState = State::Idle;
		});
	}
}
