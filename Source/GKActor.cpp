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
#include "VertexAnimator.h"
#include "Walker.h"

GKActor::GKActor() : Actor(),
	mActorType(ActorType::Prop)
{
	// Add mesh renderer.
	mMeshActor = new Actor();
	mMeshRenderer = mMeshActor->AddComponent<MeshRenderer>();
	
	// Add vertex animation player.
	mVertexAnimator = mMeshActor->AddComponent<VertexAnimator>();
	
	// Add GAS player.
	mGasPlayer = AddComponent<GasPlayer>();
}

GKActor::GKActor(const std::string& identifier) : Actor(),
	mIdentifier(identifier),
	mActorType(ActorType::Actor)
{
	// Create mesh actor with
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
	mWalker->SetWalkMeshActor(mMeshActor);
}

void GKActor::SetHeading(const Heading& heading)
{
	SetRotation(Quaternion(Vector3::UnitY, heading.ToRadians()));
}

Heading GKActor::GetHeading() const
{
	return Heading::FromQuaternion(GetRotation());
}

std::string GKActor::GetModelName() const
{
	if(mMeshRenderer == nullptr) { return std::string(); }
	
	Model* model = mMeshRenderer->GetModel();
	if(model == nullptr) { return std::string(); }
	
	return model->GetNameNoExtension();
}

void GKActor::StartAnimation(VertexAnimation* anim, int framesPerSecond, bool allowMove)
{
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
	
	// Move model to actor position.
	SetMeshToActorPositionUsingAnim(anim, framesPerSecond);
	
	// Start the animation.
	mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this));
}

void GKActor::StartAnimation(VertexAnimation* anim, int framesPerSecond, Vector3 pos, Heading heading)
{
	// Absolute anims are always move anims?
	mVertexAnimAllowMove = true;
	
	// Set the 3D model's position and heading.
	mMeshRenderer->GetOwner()->SetPosition(pos);
	mMeshRenderer->GetOwner()->SetRotation(heading.ToQuaternion());
	
	// Start the animation.
	mVertexAnimator->Start(anim, framesPerSecond, std::bind(&GKActor::OnVertexAnimationStopped, this));
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
	/*
    // Set appropriate gas to play.
    switch(type)
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
	*/
}

void GKActor::StartCustomFidget(GAS* gas)
{
	//mGasPlayer->SetGas(gas);
}

void GKActor::WalkToAnimationStart(Animation* anim, WalkerBoundary* walkerBoundary, std::function<void()> finishCallback)
{
	// Need a walker and walker boundary for any of this to work.
	if(mWalker == nullptr || walkerBoundary == nullptr) { return; }
	
	// Retrieve vertex animation that this actor's model will play on first frame of the given animation.
	// It's possible for this to be null if the actor's model is not involved in the animation! This is probably a developer error.
	if(anim != nullptr)
	{
		VertexAnimation* vertexAnim = anim->GetVertexAnimationOnFrameForModel(0, GetModelName());
		if(vertexAnim != nullptr)
		{
			VertexAnimationTransformPose transformPose = vertexAnim->SampleTransformPose(0.0f, 15, mCharConfig->hipAxesMeshIndex);
			
			// Grab position/heading from first frame of the animation.
			Vector3 walkPos = vertexAnim->SampleVertexPosition(0.0f, 15, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
			walkPos = transformPose.GetLocalTransformMatrix().TransformPoint(walkPos);
			walkPos.SetY(GetPosition().GetY());
			
			Heading heading = Heading::FromQuaternion(transformPose.mLocalRotation * Quaternion(Vector3::UnitY, Math::kPi));
			
			// Walk to that position/heading.
			mWalker->WalkTo(walkPos, heading, walkerBoundary, finishCallback);
		}
	}
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
	if(mWalker != nullptr)
	{
		mWalker->SnapWalkActorToFloor();
	}
	
	// Actor follows mesh during animation.
	if(mVertexAnimator->IsPlaying() || (mWalker != nullptr && mWalker->IsWalking()))
	{
		SetActorToMeshPosition(true);
		SetActorToMeshRotation(true);
	}
	
	/*
	// Pause any fidgets while walker is going.
	if(mGasPlayer != nullptr && mWalker != nullptr)
	{
		mGasPlayer->SetPaused(mWalker->IsWalking());
	}
	*/
}

void GKActor::OnVertexAnimationStopped()
{
	// On anim stop, if vertex anim is not allowed to move actor position,
	// we must revert actor back to position when anim started.
	if(!mVertexAnimAllowMove)
	{
		// Move actor to original position/rotation.
		SetPosition(mStartVertexAnimPosition);
		SetRotation(mStartVertexAnimRotation);
		
		// Move mesh to position as well.
		mMeshActor->SetPosition(mStartVertexAnimPosition);
		mMeshActor->SetRotation(GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));
		
		// Offset mesh to be positioned at actor position.
		SetMeshToActorPosition(true);
	}
}

void GKActor::SetMeshToActorPosition(bool useMeshPosOffset)
{
	// Move mesh to actor's position.
	mMeshActor->SetPosition(GetPosition());
	
	// The 3D mesh's graphical position does not always align to the mesh actor's position.
	// If desired, we can correct for this to ensure that the mesh's graphical position matches the actor.
	if(useMeshPosOffset && mCharConfig != nullptr)
	{
		// Get hip vertex pos based on values provided by character config. This is in the mesh's local space.
		Vector3 hipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
		
		// Convert hip vertex pos to world space.
		// To do this, we multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
		Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetLocalTransformMatrix()).TransformPoint(hipPos);
		
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
		Vector3 worldHipPos = (GetTransform()->GetLocalToWorldMatrix() * startHipTransform.GetLocalTransformMatrix()).TransformPoint(startHipPos);
		
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

void GKActor::SetActorToMeshPosition(bool useMeshPosOffset)
{
	if(useMeshPosOffset && mCharConfig != nullptr)
	{
		// Get hip vertex pos based on values provided by character config. This is in the mesh's local space.
		Vector3 hipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
		
		// Convert hip vertex pos to world space.
		// To do this, we multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
		Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetLocalTransformMatrix()).TransformPoint(hipPos);
		
		// The hip pos is usually higher up on the mesh (about hip height).
		// We want to NOT take into account the vertical height.
		worldHipPos.SetY(GetPosition().GetY());
		
		// Just put the actor at that world hip pos!
		SetPosition(worldHipPos);
	}
	else
	{
		SetPosition(mMeshActor->GetPosition());
	}
}

void GKActor::SetActorToMeshRotation(bool useMeshPosOffset)
{
	if(mCharConfig != nullptr)
	{
		// Get hip rotation and transform to world space.
		Quaternion rotation = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetLocalTransformMatrix().GetRotation();
		rotation = mMeshActor->GetTransform()->LocalToWorldRotation(rotation);
		
		// Per usual, ALSO flip 180 degrees for humanoid actors.
		SetRotation(rotation * Quaternion(Vector3::UnitY, Math::kPi));
	}
	
	/*
	if(useMeshPosOffset && mCharConfig != nullptr)
	{
		// Get hip vertex pos based on values provided by character config. This is in the mesh's local space.
		Vector3 hipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
		
		// Convert hip vertex pos to world space.
		// To do this, we multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
		Vector3 worldHipPos = (mMeshActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetLocalTransformMatrix()).TransformPoint(hipPos);
		
		// The hip pos is usually higher up on the mesh (about hip height).
		// We want to NOT take into account the vertical height.
		worldHipPos.SetY(GetPosition().GetY());
		
		// Just put the actor at that world hip pos!
		SetPosition(worldHipPos);
	}
	else
	{
		
	}
	*/
}
