//
// Walker.cpp
//
// Clark Kromenaker
//
#include "Walker.h"

#include "Actor.h"
#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Services.h"
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
	// Get position and rotation to modify.
	Vector3 position = mOwner->GetPosition();
	Quaternion rotation = mOwner->GetRotation();
	
	// If we have a path, follow it.
	if(mPath.size() > 0)
	{
		// Figure out where to move next.
		// If we're near that spot, move on to the next spot.
		Vector3 toNext = mPath[0] - position;
		if(toNext.GetLengthSq() < 1.0f)
		{
			mPath.erase(mPath.begin());
			if(mPath.size() <= 0)
			{
				StopWalk();
			}
			else
			{
				toNext = mPath[0] - position;
			}
		}
		
		// Convert to pure direction.
		Vector3 dir = toNext;
		dir.Normalize();
		
		// Update position in the direction of the goal at a certain speed.
		position += dir * 25.0f * deltaTime;
		
		// What angle do I need to rotate to face the direction to the target?
		float angle = Math::Acos(Vector3::Dot(mOwner->GetForward(), dir));
		if(Math::ToDegrees(angle) > 1.0f)
		{
			// Which way do I rotate to get to direction I want?
			Vector3 cross = Vector3::Cross(mOwner->GetForward(), dir);
			float rotateDirection = cross.GetY();
			if(!Math::IsZero(rotateDirection))
			{
				rotateDirection /= Math::Abs(rotateDirection);
			}
			
			if(Math::IsZero(rotateDirection))
			{
				std::cout << "Whoops." << std::endl;
			}
			
			float angleOfRotation = rotateDirection * 10.0f * deltaTime;
			angleOfRotation = Math::Min(angleOfRotation, angle);
			
			rotation *= Quaternion(Vector3::UnitY, angleOfRotation);
		}
	}
	
	/*
	if(mState == State::Idle) { StartWalk(); }
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_G))
	{
		ContinueWalk();
	}
	*/
	
	// Stay attached to the floor.
	Scene* scene = GEngine::inst->GetScene();
	if(scene != nullptr)
	{
		position.SetY(scene->GetFloorY(position));
	}
	
	// Keep the main walker offset by the aid.
	if(mWalkMeshTransform != nullptr && mWalkAidMeshRenderer != nullptr)
	{
		auto meshes = mWalkAidMeshRenderer->GetMeshes();
		if(meshes.size() > 0)
		{
			Vector3 aidMeshLocalPos = meshes[0]->GetLocalTransformMatrix().GetTranslation();
			mWalkMeshTransform->SetPosition(aidMeshLocalPos);
		}
		
		// TODO: GK3 CHARACTERS.TXT defines hip axes mesh/group/point.
		// For Gabe, this is 5/0/125.
		// During walk anims, this is a point that is ALWAYS at origin (0, 0, 0).
		// Maybe there's some way to make use of this to "fix" walk anim issues?
		/*
		MeshRenderer* r = mWalkMeshTransform->GetOwner()->GetComponent<MeshRenderer>();
		Submesh* submesh = r->GetMeshes()[5]->GetSubmesh(0);
		Vector3 hipPos = submesh->GetVertexPosition(125);
		
		Vector3 worldHipPos = mWalkMeshTransform->LocalToWorldPoint(hipPos);
		Debug::DrawLine(Vector3::Zero, worldHipPos, Color32::Blue);
		*/
	}
	
	// Set updated position.
	mOwner->SetPosition(position);
	mOwner->SetRotation(rotation);
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
