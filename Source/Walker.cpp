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
#include "Heading.h"
#include "Matrix4.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Services.h"
#include "Vector3.h"
#include "WalkerBoundary.h"

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

void Walker::OnUpdate(float deltaTime)
{
	Actor* owner = GetOwner();
	
	// Get position and rotation to modify.
	Vector3 position = owner->GetPosition();
	Quaternion rotation = owner->GetRotation();
	
	// Which direction should we turn to face? None at first.
	Vector3 turnToFaceDir;
	
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
				// If no desired heading was specified, we can do the callback right now.
				if(!mHasDesiredFacingDir && mFinishedPathCallback != nullptr)
				{
					mFinishedPathCallback();
					mFinishedPathCallback = nullptr;
				}
				StopWalk();
			}
			else
			{
				toNext = mPath[0] - position;
			}
		}
		
		if(mPath.size() > 0)
		{
			// Convert to pure direction.
			Vector3 dir = toNext;
			dir.Normalize();
			
			// Update position in the direction of the goal at a certain speed.
			position += dir * mMoveSpeed * deltaTime;
			
			// Want to turn towards direction to next path node.
			turnToFaceDir = dir;
		}
	}
	
	// If pathing logic doesn't specify a facing direction, we use the one specified, if any.
	if(turnToFaceDir == Vector3::Zero && mHasDesiredFacingDir)
	{
		turnToFaceDir = mDesiredFacingDir;
	}
	
	// If not zero, it means we want to rotate towards some direction.
	if(turnToFaceDir != Vector3::Zero)
	{
		// What angle do I need to rotate to face the direction to the target?
		float angle = Math::Acos(Vector3::Dot(owner->GetForward(), turnToFaceDir));
		if(Math::ToDegrees(angle) > 1.0f)
		{
			// Which way do I rotate to get to facing direction I want?
			// Can use y-axis of cross product to determine this.
			Vector3 cross = Vector3::Cross(owner->GetForward(), turnToFaceDir);
			
			// If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
			// In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
			float rotateDirection = 1.0f;
			if(!Math::IsZero(cross.GetY()))
			{
				rotateDirection = cross.GetY() / Math::Abs(cross.GetY());
			}
			
			// Calculate an angle we are going to rotate by, moving us toward our desired facing.
			float angleOfRotation = rotateDirection * mRotateSpeed * deltaTime;
			angleOfRotation = Math::Min(angleOfRotation, angle);
			
			// Update our rotation by this angle amount.
			rotation *= Quaternion(Vector3::UnitY, angleOfRotation);
		}
		else
		{
			// If within acceptable range of our desired facing direction, AND no more path, clear desired heading.
			if(mPath.size() <= 0 && mHasDesiredFacingDir)
			{
				mHasDesiredFacingDir = false;
				
				// At this point, we've REALLY finished our path.
				if(mFinishedPathCallback != nullptr)
				{
					mFinishedPathCallback();
					mFinishedPathCallback = nullptr;
				}
			}
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
	owner->SetPosition(position);
	owner->SetRotation(rotation);
}

bool Walker::WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	return WalkTo(position, Heading::None, walkerBoundary, finishCallback);
}

bool Walker::WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	// Save finish callback.
	mFinishedPathCallback = finishCallback;
	
	// Save desired facing direction.
	if(heading.IsValid())
	{
		mHasDesiredFacingDir = true;
		mDesiredFacingDir = heading.ToVector();
	}
	else
	{
		mHasDesiredFacingDir = false;
	}
	
	// Can't path without a valid walker boundary to define the area.
	if(walkerBoundary == nullptr)
	{
		if(mFinishedPathCallback != nullptr)
		{
			mFinishedPathCallback();
			mFinishedPathCallback = nullptr;
		}
		return false;
	}
	
	// Find a path from current position to target position, populating our path vector.
	if(walkerBoundary->FindPath(GetOwner()->GetPosition(), position, mPath))
	{
		//TODO: Make debug output of paths optional.
		{
			Vector3 prev = mPath[0];
			for(int i = 1; i < mPath.size(); i++)
			{
				Debug::DrawLine(prev, mPath[i], Color32::Red, 10.0f);
				prev = mPath[i];
			}
		}
		
		StartWalk();
		return true;
	}
	else
	{
		std::cout << "No path!" << std::endl;
		if(mFinishedPathCallback != nullptr)
		{
			mFinishedPathCallback();
			mFinishedPathCallback = nullptr;
		}
		return false;
	}
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
