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
    // Update heading of this actor.
	GKObject::SetHeading(heading);
    
    // Update mesh to match.
    if(mActorType == ActorType::Actor)
    {
        mMeshActor->SetRotation(GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));
    }
    else
    {
        mMeshActor->SetRotation(GetRotation());
    }
}

std::string GKActor::GetModelName() const
{
	if(mMeshRenderer == nullptr) { return std::string(); }
	Model* model = mMeshRenderer->GetModel();
	if(model == nullptr) { return std::string(); }
	return model->GetNameNoExtension();
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
	walkPos.y = GetPosition().y;
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
    SyncActorTransformToMesh();
    
    /*
	if(mMeshRenderer != nullptr)
	{
		mMeshRenderer->DebugDrawAABBs();
	}
    */
}
