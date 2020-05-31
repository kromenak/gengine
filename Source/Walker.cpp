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

void Walker::SnapToFloor()
{
	Scene* scene = GEngine::inst->GetScene();
	if(scene != nullptr)
	{
		Vector3 pos = GetOwner()->GetPosition();
		pos.SetY(scene->GetFloorY(pos));
		GetOwner()->SetPosition(pos);
	}
}

bool Walker::WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	return WalkTo(position, Heading::None, walkerBoundary, finishCallback);
}

bool Walker::WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	// Save destination.
	mDestination = position;
	
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
		if(!mHasDesiredFacingDir)
		{
			if(mFinishedPathCallback != nullptr)
			{
				mFinishedPathCallback();
				mFinishedPathCallback = nullptr;
			}
			return false;
		}
		return true;
	}
	
	// If already at position...don't move! "At position" is...less than 10 units? Maybe?
	float distToPositionSq = (GetOwner()->GetPosition() - position).GetLengthSq();
	if(distToPositionSq < kAtNodeDistSq)
	{
		// If there's a desired facing direction, then technically the walker still has work to do - it has to turn the actor.
		// So, return true if desired facing, false otherwise.
		if(!mHasDesiredFacingDir)
		{
			if(mFinishedPathCallback != nullptr)
			{
				mFinishedPathCallback();
				mFinishedPathCallback = nullptr;
			}
			return false;
		}
		return true;
	}
	
	// Find a path from current position to target position, populating our path vector.
	if(walkerBoundary->FindPath(GetOwner()->GetPosition(), position, mPath))
	{
		//TODO: Make debug output of paths optional.
		{
			Vector3 prev = mPath.back();
			for(int i = static_cast<int>(mPath.size()) - 2; i >= 0; i--)
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

void Walker::OnUpdate(float deltaTime)
{
	Actor* walkActor = GetOwner();
	
	// Which direction should we turn to face? None at first.
	Vector3 turnToFaceDir;
	
	// If we have a path, follow it.
	if(mPath.size() > 0)
	{
		Debug::DrawLine(walkActor->GetPosition(), mPath.back(), Color32::White);
		
		// Figure out where to move next.
		// If we're near that spot, move on to the next spot.
		Vector3 toNext = mPath.back() - walkActor->GetPosition();
		if(toNext.GetLengthSq() < kAtNodeDistSq)
		{
			/*
			// We are approaching the final node in the path - set our position exactly.
			if(mPath.size() == 1)
			{
				walkActor->SetPosition(mPath.back());
			}
			*/
			
			mPath.pop_back();
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
				toNext = mPath.back() - walkActor->GetPosition();
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
		float angle = Math::Acos(Vector3::Dot(walkActor->GetForward(), turnToFaceDir));
		if(angle > kAtHeadingRadians)
		{
			// Which way do I rotate to get to facing direction I want?
			// Can use y-axis of cross product to determine this.
			Vector3 cross = Vector3::Cross(walkActor->GetForward(), turnToFaceDir);
			
			// If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
			// In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
			float rotateDirection = 1.0f;
			if(!Math::IsZero(cross.GetY()))
			{
				rotateDirection = cross.GetY() / Math::Abs(cross.GetY());
			}
			
			// Determine how much we'll rotate this frame.
			// If it's enough to where our angle will be less than the "at heading" rotation, just set heading exactly.
			float rotateSpeed = mPath.empty() ? kTurnSpeed : kWalkTurnSpeed;
			float angleOfRotation = rotateSpeed * deltaTime;
			if(angle - angleOfRotation <= kAtHeadingRadians)
			{
				angleOfRotation = angle;
			}
			
			// Factor direction into angle.
			angleOfRotation *= rotateDirection;
			
			// Update our rotation by this angle amount.
			Transform* meshTransform = mWalkMeshActor->GetTransform();
			meshTransform->RotateAround(walkActor->GetPosition(), Vector3::UnitY, angleOfRotation);
			walkActor->GetTransform()->Rotate(Vector3::UnitY, angleOfRotation);
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

void Walker::StartWalk()
{
	if(mState == State::Idle)
	{
		Animation* anim = mCharConfig->walkStartAnim;
		
		/*
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
		*/
		 
		mState = State::Start;
		GEngine::inst->GetScene()->GetAnimator()->Start(anim, true, false, std::bind(&Walker::ContinueWalk, this));
	}
}

void Walker::ContinueWalk()
{
	mState = State::Loop;
	GEngine::inst->GetScene()->GetAnimator()->Start(mCharConfig->walkLoopAnim, true, false, std::bind(&Walker::ContinueWalk, this));
}

void Walker::StopWalk()
{
	if(mState != State::End)
	{
		GEngine::inst->GetScene()->GetAnimator()->Stop(mCharConfig->walkStartAnim);
		GEngine::inst->GetScene()->GetAnimator()->Stop(mCharConfig->walkLoopAnim);
		
		mState = State::End;
		GEngine::inst->GetScene()->GetAnimator()->Start(mCharConfig->walkStopAnim, true, false, [this]() -> void {
			mState = State::Idle;
			//mWalkActor->StartFidget(GKActor::FidgetType::Idle);
		});
	}
}
