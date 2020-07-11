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
#include "StringUtil.h"
#include "Vector3.h"
#include "WalkerBoundary.h"

TYPE_DEF_CHILD(Component, Walker);

Walker::Walker(Actor* owner) : Component(owner),
	mGKOwner(static_cast<GKActor*>(owner))
{
	
}

void Walker::SetCharacterConfig(const CharacterConfig& characterConfig)
{
	mCharConfig = &characterConfig;
}

void Walker::SnapToFloor()
{
	Scene* scene = GEngine::Instance()->GetScene();
	if(scene != nullptr)
	{
		Vector3 pos = GetOwner()->GetPosition();
		pos.y = scene->GetFloorY(pos);
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
			OnWalkToFinished();
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
			OnWalkToFinished();
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

bool Walker::WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	mWalkToSeeTarget = targetName;
	mWalkToSeeTargetPosition = targetPosition;
	std::cout << "Target is " << targetName << std::endl;
	
	// Check whether thing is already in view.
	// If so, we don't even need to walk (but may need to turn-to-face).
	Vector3 facingDir;
	if(IsWalkToSeeTargetInView(facingDir))
	{
		// Be sure to save finish callback in this case - it usually happens in walk to.
		mFinishedPathCallback = finishCallback;
		
		// No need to walk, but do turn to face the thing.
		mHasDesiredFacingDir = true;
		mDesiredFacingDir = facingDir;
		return true;
	}
	else
	{
		return WalkTo(targetPosition, walkerBoundary, finishCallback);
	}
}

void Walker::OnUpdate(float deltaTime)
{
	// If we have a "walk to see" target, check whether it has come into view.
	bool stopWalkPrematurely = false;
	if(!mWalkToSeeTarget.empty())
	{
		Vector3 facingDir;
		if(IsWalkToSeeTargetInView(facingDir))
		{
			stopWalkPrematurely = true;
			
			// When doing a "walk to see," we want the actor to turn to face.
			mHasDesiredFacingDir = true;
			mDesiredFacingDir = facingDir;
		}
	}
	
	// Which direction should we turn to face? None at first.
	Vector3 turnToFaceDir;
	
	// If we have a path, follow it.
	if(mPath.size() > 0)
	{
		Debug::DrawLine(mGKOwner->GetPosition(), mPath.back(), Color32::White);
		
		// Figure out where to move next.
		// If we're near that spot, move on to the next spot.
		Vector3 toNext = mPath.back() - mGKOwner->GetPosition();
		if(toNext.GetLengthSq() < kAtNodeDistSq || stopWalkPrematurely)
		{
			/*
			// We are approaching the final node in the path - set our position exactly.
			if(mPath.size() == 1)
			{
				mGKOwner->SetPosition(mPath.back());
			}
			*/
			
			mPath.pop_back();
			if(mPath.size() <= 0)
			{
				// Stop walk anim.
				StopWalk();
				
				// If no desired heading was specified, we can do the callback right now.
				if(!mHasDesiredFacingDir)
				{
					OnWalkToFinished();
				}
			}
			else
			{
				toNext = mPath.back() - mGKOwner->GetPosition();
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
		float angle = Math::Acos(Vector3::Dot(mGKOwner->GetForward(), turnToFaceDir));
		if(angle > kAtHeadingRadians)
		{
			// Which way do I rotate to get to facing direction I want?
			// Can use y-axis of cross product to determine this.
			Vector3 cross = Vector3::Cross(mGKOwner->GetForward(), turnToFaceDir);
			
			// If y-axis is zero, it means vectors are parallel (either exactly facing or exactly NOT facing).
			// In that case, 1.0f default is fine. Otherwise, we want either 1.0f or -1.0f.
			float rotateDirection = 1.0f;
			if(!Math::IsZero(cross.y))
			{
				rotateDirection = cross.y / Math::Abs(cross.y);
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
			Transform* meshTransform = mGKOwner->GetMeshRenderer()->GetOwner()->GetTransform();
			meshTransform->RotateAround(mGKOwner->GetPosition(), Vector3::UnitY, angleOfRotation);
			mGKOwner->GetTransform()->Rotate(Vector3::UnitY, angleOfRotation);
		}
		else
		{
			// If within acceptable range of our desired facing direction, AND no more path, clear desired heading.
			if(mPath.size() <= 0 && mHasDesiredFacingDir)
			{
				mHasDesiredFacingDir = false;
				
				// At this point, we've REALLY finished our path.
				OnWalkToFinished();
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
			if(cross.y > 0)
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
		GEngine::Instance()->GetScene()->GetAnimator()->Start(anim, true, false, std::bind(&Walker::ContinueWalk, this));
	}
}

void Walker::ContinueWalk()
{
	mState = State::Loop;
	GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkLoopAnim, true, false, std::bind(&Walker::ContinueWalk, this));
}

void Walker::StopWalk()
{
	if(mState != State::End)
	{
		GEngine::Instance()->GetScene()->GetAnimator()->Stop(mCharConfig->walkStartAnim);
		GEngine::Instance()->GetScene()->GetAnimator()->Stop(mCharConfig->walkLoopAnim);
		
		mState = State::End;
		GEngine::Instance()->GetScene()->GetAnimator()->Start(mCharConfig->walkStopAnim, true, false, [this]() -> void {
			mState = State::Idle;
			//mowner->StartFidget(GKActor::FidgetType::Idle);
		});
	}
}

void Walker::OnWalkToFinished()
{
	// Make sure state variables are cleared.
	mPath.clear();
	mHasDesiredFacingDir = false;
	mWalkToSeeTarget.clear();
	
	// Call finished callback.
	if(mFinishedPathCallback != nullptr)
	{
		mFinishedPathCallback();
		mFinishedPathCallback = nullptr;
	}
}

bool Walker::IsWalkToSeeTargetInView(Vector3& outTurnToFaceDir)
{
	Vector3 headPos = mGKOwner->GetHeadPosition();
	Debug::DrawLine(headPos, mWalkToSeeTargetPosition, Color32::Magenta);
	
	// Create ray from head in direction of target position.
	Vector3 dir = (mWalkToSeeTargetPosition - headPos).Normalize();
	Ray ray(headPos, dir);
	
	// If hit the target with the ray, it must be in view.
	SceneCastResult result = GEngine::Instance()->GetScene()->Raycast(ray, false, mGKOwner);
	if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, mWalkToSeeTarget))
	{
		// Convert ray direction to a "facing" direction,
		dir.y = 0.0f;
		outTurnToFaceDir = dir.Normalize();
		return true;
	}
	return false;
}
