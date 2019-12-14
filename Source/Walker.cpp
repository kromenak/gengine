//
// Walker.cpp
//
// Clark Kromenaker
//
#include "Walker.h"

#include "Actor.h"
#include "Animator.h"
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
	mCharConfig = &characterConfig;
}

void Walker::SnapWalkActorToFloor()
{
	Scene* scene = GEngine::inst->GetScene();
	if(scene != nullptr)
	{
		Vector3 pos = mWalkActor->GetPosition();
		pos.SetY(scene->GetFloorY(pos));
		mWalkActor->SetPosition(pos);
	}
}

void Walker::SnapToWalkActor()
{
	GetOwner()->SetPosition(mWalkActor->GetPosition());
	GetOwner()->SetRotation(mWalkActor->GetRotation());
}

void Walker::OnUpdate(float deltaTime)
{
	// Which direction should we turn to face? None at first.
	Vector3 turnToFaceDir;
	
	// If we have a path, follow it.
	if(mPath.size() > 0)
	{
		// Figure out where to move next.
		// If we're near that spot, move on to the next spot.
		Vector3 toNext = mPath[0] - mWalkActor->GetPosition();
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
				toNext = mPath[0] - mWalkActor->GetPosition();
			}
		}
		
		// Convert to pure direction.
		Vector3 dir = toNext;
		dir.Normalize();
		
		// Want to turn towards direction to next path node.
		turnToFaceDir = dir;
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
		float angle = Math::Acos(Vector3::Dot(mWalkActor->GetForward(), turnToFaceDir));
		if(Math::ToDegrees(angle) > 1.0f)
		{
			// Which way do I rotate to get to facing direction I want?
			// Can use y-axis of cross product to determine this.
			Vector3 cross = Vector3::Cross(mWalkActor->GetForward(), turnToFaceDir);
			
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
			Quaternion rotation = mWalkMeshActor->GetRotation();
			rotation *= Quaternion(Vector3::UnitY, angleOfRotation);
			mWalkMeshActor->SetRotation(rotation * Quaternion(Vector3::UnitY, Math::kPi));
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
		Debug::DrawLine(GetOwner()->GetPosition(), position, Color32::Blue, 10.0f);
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
		Animation* anim = mCharConfig->walkStartAnim;
		
		// Uhhh, this is trying to do left/right start turn anims...but it's pretty bad at this point :P
		Vector3 toNext = mPath.back() - GetOwner()->GetPosition();
		toNext.Normalize();
		if(Vector3::Dot(GetOwner()->GetForward(), toNext) < 0.6f)
		{
			Vector3 cross = Vector3::Cross(GetOwner()->GetForward(), toNext);
			if(cross.GetY() > 0)
			{
				anim = mCharConfig->walkStartTurnRightAnim;
			}
			else
			{
				anim = mCharConfig->walkStartTurnLeftAnim;
			}
		}
		mState = State::Start;
		GEngine::inst->GetScene()->GetAnimator()->Start(anim, true, std::bind(&Walker::ContinueWalk, this));
	}
}

void Walker::ContinueWalk()
{
	mState = State::Loop;
	GEngine::inst->GetScene()->GetAnimator()->Start(mCharConfig->walkLoopAnim, true, std::bind(&Walker::ContinueWalk, this));
}

void Walker::StopWalk()
{
	if(mState != State::End)
	{
		GEngine::inst->GetScene()->GetAnimator()->Stop(mCharConfig->walkStartAnim);
		GEngine::inst->GetScene()->GetAnimator()->Stop(mCharConfig->walkLoopAnim);
		
		mState = State::End;
		GEngine::inst->GetScene()->GetAnimator()->Start(mCharConfig->walkStopAnim, true, [this]() -> void {
			mState = State::Idle;
			mWalkActor->StartFidget(GKActor::FidgetType::Idle);
		});
	}
}
