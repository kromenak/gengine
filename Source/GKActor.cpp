//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include <sstream>

#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Services.h"
#include "VertexAnimator.h"
#include "Walker.h"

GKActor::GKActor() : GKObject(),
	mActorType(ActorType::Prop)
{
	// Create mesh actor with mesh renderer on it.
	mMeshActor = new Actor();
	mMeshRenderer = mMeshActor->AddComponent<MeshRenderer>();
	
	// Create animation player on the same object as the mesh renderer.
	mVertexAnimator = mMeshActor->AddComponent<VertexAnimator>();
	
	// GasPlayer will go on the actor itself.
	mGasPlayer = AddComponent<GasPlayer>();
}

GKActor::GKActor(const std::string& identifier) : GKObject(),
	mIdentifier(identifier),
	mActorType(ActorType::Actor)
{
	// Create mesh actor with mesh renderer on it.
	mMeshActor = new Actor();
	mMeshRenderer = mMeshActor->AddComponent<MeshRenderer>();
	
	// Create animation player on the same object as the mesh renderer.
    mVertexAnimator = mMeshActor->AddComponent<VertexAnimator>();
	
	// GasPlayer will go on the actor itself.
    mGasPlayer = AddComponent<GasPlayer>();
	
	// Get config for this character.
	CharacterConfig& config = Services::Get<CharacterManager>()->GetCharacterConfig(mIdentifier);
	mCharConfig = &config;
	
	// Create and configure face controller.
	mFaceController = AddComponent<FaceController>();
	mFaceController->SetCharacterConfig(config);
	
	// Add walker and configure it.
	mWalker = AddComponent<Walker>();
	mWalker->SetCharacterConfig(config);
}

void GKActor::SetHeading(const Heading& heading)
{
	GKObject::SetHeading(heading);
	SetMeshToActorRotation();
}

std::string GKActor::GetModelName() const
{
	if(mMeshRenderer == nullptr) { return std::string(); }
	
	Model* model = mMeshRenderer->GetModel();
	if(model == nullptr) { return std::string(); }
	
	return model->GetNameNoExtension();
}

void GKActor::StartAnimation(VertexAnimation* anim, int framesPerSecond, bool allowMove, float time, bool fromGas)
{
	// Don't let a GAS anim override a non-GAS anim.
	if(fromGas && mVertexAnimator->IsPlaying()) { return; }
	
	// Stop any playing animation before starting a new one.
	if(mVertexAnimator->IsPlaying())
	{
		mVertexAnimator->Stop(nullptr);
	}
	
	// Save whether this anim is allowed to move the actor.
	mVertexAnimAllowMove = allowMove;
	
	// If not allowed to move, save the start pos/rot so we can revert when the anim stops.
	// In GK3, non-move anims STILL move the actor during the animation, but reverts the position at the end.
	if(!allowMove)
	{
		mStartVertexAnimPosition = GetPosition();
		mStartVertexAnimRotation = GetRotation();
	}
	
	// Move mesh to actor's rotation.
	if(mActorType == ActorType::Actor)
	{
		mMeshActor->SetRotation(GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));
	}
	else
	{
		mMeshActor->SetRotation(GetRotation());
	}
	
	// Move mesh to actor position.
	SetMeshToActorPositionUsingAnim(anim, framesPerSecond);
	
	// If this is not a GAS anim, pause any running GAS.
	if(!fromGas)
	{
		mGasPlayer->Pause();
	}
	
	// Start the animation.
	mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this), time);
}

void GKActor::StartAbsoluteAnimation(VertexAnimation* anim, int framesPerSecond, Vector3 pos, Heading heading, float time, bool fromGas)
{
	// Don't let a GAS anim override a non-GAS anim.
	if(fromGas && mVertexAnimator->IsPlaying()) { return; }
	
	// Absolute anims are always move anims?
	mVertexAnimAllowMove = true;
	
	// Set the 3D model's position and heading.
	mMeshActor->SetPosition(pos);
	mMeshActor->SetRotation(heading.ToQuaternion());
	
	// If this is not a GAS anim, pause any running GAS.
	if(!fromGas)
	{
		mGasPlayer->Pause();
	}
	
	// Start the animation.
	mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this), time);
}

void GKActor::StopAnimation(VertexAnimation* anim)
{
	// Vertex animator will only stop if specified anim is one currently playing.
	// But keep in mind passing null will stop any anim!
	mVertexAnimator->Stop(anim);
}

void GKActor::SampleAnimation(VertexAnimation* anim, int frame)
{
	mVertexAnimator->Sample(anim, frame);
}

void GKActor::StartFidget(FidgetType type)
{
    // Set appropriate gas to play.
    switch(type)
    {
	case FidgetType::None:
	case FidgetType::Custom: // Can only be started via StartCustomFidget.
		mGasPlayer->SetGas(nullptr);
		break;
		
	case FidgetType::Idle:
		mGasPlayer->SetGas(mIdleFidget);
		break;
		
	case FidgetType::Talk:
		mGasPlayer->SetGas(mTalkFidget);
		break;
		
	case FidgetType::Listen:
		mGasPlayer->SetGas(mListenFidget);
		break;
    }
	
	// Save type we're playing.
	mCurrentFidget = type;
	
	// Play it!
	mGasPlayer->Play();
}

void GKActor::StartCustomFidget(GAS* gas)
{
	// Set and play.
	mCurrentFidget = FidgetType::Custom;
	mGasPlayer->SetGas(gas);
	mGasPlayer->Play();
}

void GKActor::StopFidget()
{
	mGasPlayer->Pause();
}

void GKActor::DumpPosition()
{
	std::stringstream ss;
	ss << "actor '" << GetNoun() << "' ";
	ss << "h=" << GetHeading() << ", ";
	ss << "pos=" << GetPosition();
	Services::GetReports()->Log("Dump", ss.str());
}

void GKActor::TurnTo(const Heading& heading, std::function<void()> finishCallback)
{
	mWalker->WalkTo(GetPosition(), heading, nullptr, finishCallback);
}

void GKActor::WalkTo(const Vector3& position, const Heading& heading, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	mWalker->WalkTo(position, heading, walkerBoundary, finishCallback);
}

void GKActor::WalkTo(const Vector3& position, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	mWalker->WalkTo(position, walkerBoundary, finishCallback);
}

void GKActor::WalkToAnimationStart(Animation* anim, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	// Need a valid anim.
	if(anim == nullptr) { return; }
	
	// Need a walker and walker boundary for any of this to work.
	if(mWalker == nullptr || walkerBoundary == nullptr) { return; }
	
	// GOAL: walk the actor to the initial position of this anim.
	// Retrieve vertex animation from animation that matches this actor's model.
	// If no vertex anim exists, we can't walk to animation start! This is probably a developer error.
	VertexAnimation* vertexAnim = anim->GetVertexAnimationOnFrameForModel(0, GetModelName());
	if(vertexAnim == nullptr) { return; }
	
	// Grab position/heading from first frame of the animation.
	Vector3 walkPos = vertexAnim->SampleVertexPosition(0.0f, 15, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
	VertexAnimationTransformPose transformPose = vertexAnim->SampleTransformPose(0.0f, 15, mCharConfig->hipAxesMeshIndex);
	walkPos = transformPose.GetMeshToLocalMatrix().TransformPoint(walkPos);
	walkPos.SetY(GetPosition().GetY());
	Heading heading = Heading::FromQuaternion(transformPose.mLocalRotation * Quaternion(Vector3::UnitY, Math::kPi));
	
	// Walk to that position/heading.
	mWalker->WalkTo(walkPos, heading, walkerBoundary, finishCallback);
}

void GKActor::WalkToSee(const std::string& targetName, const Vector3& targetPosition, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	mWalker->WalkToSee(targetName, targetPosition, walkerBoundary, finishCallback);
}

Vector3 GKActor::GetWalkDestination() const
{
	if(mWalker == nullptr || !mWalker->IsWalking()) { return GetPosition(); }
	return mWalker->GetDestination();
}

void GKActor::SnapToFloor()
{
	if(mWalker != nullptr)
	{
		mWalker->SnapToFloor();
	}
}

Vector3 GKActor::GetHeadPosition() const
{
	/*
	if(mMeshRenderer != nullptr)
	{
		Mesh* headMesh = mMeshRenderer->GetMesh(mCharConfig->headMeshIndex);
		if(headMesh != nullptr)
		{
			Vector3 headCenter = headMesh->GetAABB().GetCenter();
			Matrix4 meshToWorld = mMeshActor->GetTransform()->GetLocalToWorldMatrix() * headMesh->GetMeshToLocalMatrix();
			return meshToWorld.TransformPoint(headCenter);
		}
	}
	*/
	
	// Can't determine exact head position! Just return position.
	Vector3 position = GetPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

void GKActor::OnActive()
{
	// My mesh becomes active when I become active.
	mMeshActor->SetActive(true);
}

void GKActor::OnInactive()
{
	// My mesh becomes inactive when I become inactive.
	mMeshActor->SetActive(false);
}

void GKActor::OnUpdate(float deltaTime)
{
	// Stay on the ground.
	SnapToFloor();
	
	// Actor follows mesh during animation.
	if(mVertexAnimator->IsPlaying() && mVertexAnimAllowMove)//|| (mWalker != nullptr && mWalker->IsWalking()))
	{
		SetActorToMeshPosition();
		SetActorToMeshRotation();
	}
	
	if(mMeshRenderer != nullptr)
	{
		//mMeshRenderer->DebugDrawAABBs();
	}
}

void GKActor::OnVertexAnimationStopped()
{
	// On anim stop, if vertex anim is not allowed to move actor position,
	// we must revert actor back to position when anim started.
	if(!mVertexAnimAllowMove)
	{
		// Move actor to original position/rotation.
		//SetPosition(mStartVertexAnimPosition);
		//SetRotation(mStartVertexAnimRotation);
		
		// Mesh position/rotation should match the actor.
		//SetMeshToActorPosition();
		//SetMeshToActorRotation();
	}
	
	// Resume fidget (well, play from beginning - gives better results).
	StartFidget(mCurrentFidget);
}

void GKActor::SetMeshToActorPosition()
{
	// Move mesh to actor's position.
	mMeshActor->SetPosition(GetPosition());
	
	// The mesh's graphical position does not always align to the mesh actor's position.
	// Ensure that the mesh's graphical position matches the actor.
	if(mCharConfig != nullptr)
	{
		// Get hip vertex pos based on values provided by character config. This is in the mesh's local space.
		Vector3 hipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
		
		// Convert hip vertex pos to world space.
		// To do this, we multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
		Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(hipPos);
		
		// The hip pos is usually higher up on the mesh (about hip height).
		// We want to NOT take into account the vertical height.
		worldHipPos.SetY(GetPosition().GetY());
		
		// Calculate offset from hip position to actor's position.
		// This is how much we want to "correct" or move the mesh actor's position so the mesh itself is positioned at the actor's position.
		Vector3 hipPosToActor = GetPosition() - worldHipPos;
		
		// Apply the offset to the mesh actor.
		mMeshActor->SetPosition(mMeshActor->GetPosition() + hipPosToActor);
	}
}

void GKActor::SetMeshToActorPositionUsingAnim(VertexAnimation* anim, int framesPerSecond)
{
	if(mCharConfig != nullptr)
	{
		// Sample the anim start for local vertex position AND the mesh's local transform for that frame.
		Vector3 startHipPos = anim->SampleVertexPosition(0.0f, framesPerSecond, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
		VertexAnimationTransformPose startHipTransform = anim->SampleTransformPose(0.0f, framesPerSecond, mCharConfig->hipAxesMeshIndex);
		
		// Calculate world hip pos using those
		Vector3 worldHipPos = (GetTransform()->GetLocalToWorldMatrix() * startHipTransform.GetMeshToLocalMatrix()).TransformPoint(startHipPos);
		
		// Calculate world position of the actor's origin.
		Vector3 worldModelOrigin = GetTransform()->GetLocalToWorldMatrix().TransformPoint(Vector3::Zero);
		
		// Make sure hip and origin Y-components are equal (no height in this calculation).
		worldHipPos.SetY(worldModelOrigin.GetY());
		
		// Calculate offset from hip position to actor's position.
		// This is how much we want to "correct" or move the mesh actor's position so the mesh itself is positioned at the actor's position.
		//Debug::DrawLine(worldModelOrigin, worldHipPos, Color32::Blue);
		Vector3 hipPosToActor = worldHipPos - worldModelOrigin;
		
		// Update walking actor's position to match the walker's position, minus that offset to counteract the animation (whew).
		mMeshActor->SetPosition(GetPosition() + hipPosToActor);
	}
}

void GKActor::SetMeshToActorRotation()
{
	// Move mesh to actor's rotation.
	if(mActorType == ActorType::Actor)
	{
		mMeshActor->SetRotation(GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));
	}
	else
	{
		mMeshActor->SetRotation(GetRotation());
	}
}

void GKActor::SetActorToMeshPosition()
{
	if(mCharConfig != nullptr)
	{
		// Get hip vertex pos based on values provided by character config. This is in the mesh's local space.
		Vector3 hipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
		
		// Convert hip vertex pos to world space.
		// To do this, we multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
		Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(hipPos);
		
		// The hip pos is usually higher up on the mesh (about hip height).
		// We want to NOT take into account the vertical height.
		worldHipPos.SetY(GetPosition().GetY());
		
		// Just put the actor at that world hip pos!
		SetPosition(worldHipPos);
	}
}

void GKActor::SetActorToMeshRotation()
{
	if(mCharConfig != nullptr)
	{
		// Get hip rotation and transform to world space.
		Quaternion rotation = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix().GetRotation();
		rotation = mMeshActor->GetTransform()->LocalToWorldRotation(rotation);
		
		// Per usual, ALSO flip 180 degrees for humanoid actors.
		SetRotation(rotation * Quaternion(Vector3::UnitY, Math::kPi));
	}
}
