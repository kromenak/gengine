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

GKActor::GKActor(const std::string& identifier) : GKProp(true),
	mIdentifier(identifier)
{
	// Get config for this character.
	CharacterConfig& config = Services::Get<CharacterManager>()->GetCharacterConfig(mIdentifier);
	mCharConfig = &config;
	
	// Create and configure face controller.
	mFaceController = AddComponent<FaceController>();
	mFaceController->SetCharacterConfig(config);
	
	// Add walker and configure it.
	mWalker = AddComponent<Walker>();
	mWalker->SetCharacterConfig(config);
    
    // Actor meshes are facing down -Z axis, but we need them to face +Z.
    mMeshLocalRotation = Quaternion(Vector3::UnitY, Math::kPi);
}

void GKActor::StartFidget(FidgetType type)
{
    switch(type)
    {
	case FidgetType::Idle:
		mGasPlayer->Play(mIdleFidget);
		break;
		
	case FidgetType::Talk:
		mGasPlayer->Play(mTalkFidget);
		break;
		
	case FidgetType::Listen:
		mGasPlayer->Play(mListenFidget);
		break;
    }
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
	if(!mWalker->IsWalking()) { return GetPosition(); }
	return mWalker->GetDestination();
}

void GKActor::SnapToFloor()
{
    mWalker->SnapToFloor();
}

Vector3 GKActor::GetHeadPosition() const
{
	// Can't determine exact head position! Just return position.
	Vector3 position = GetPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

void GKActor::OnUpdate(float deltaTime)
{
    GKProp::OnUpdate(deltaTime);
    
    MeshRenderer* dorRenderer = mWalkerDOR->GetMeshRenderer();
    Vector3 dorPos = dorRenderer->GetMesh(0)->GetSubmesh(0)->GetVertexPosition(0);
    dorPos = (mWalkerDOR->GetTransform()->GetLocalToWorldMatrix() * dorRenderer->GetMesh(0)->GetMeshToLocalMatrix()).TransformPoint(dorPos);
    Debug::DrawLine(dorPos, dorPos + Vector3::UnitY * 20.0f, Color32::Green);
    
    {
        if(mWalkerDOR->GetVertexAnimator()->IsPlaying())
        {
           
        }
        
        // Track mesh movement since last frame and translate actor to match.
        Vector3 meshPosition = GetMeshPosition();
        Vector3 meshPositionChange = meshPosition - mLastMeshPos;
        GetTransform()->Translate(meshPositionChange);
        mLastMeshPos = meshPosition;
        
        // Same idea for rotation.
        Quaternion meshRotation = GetMeshRotation();
        Quaternion meshRotationChange = Quaternion::Diff(meshRotation, mLastMeshRotation);
        GetTransform()->Rotate(meshRotationChange);
        mLastMeshRotation = meshRotation;
    }
    
	// Stay on the ground.
	SnapToFloor();
}

void GKActor::OnVertexAnimationStart(const VertexAnimParams& animParams)
{
    // For relative anims, move the model so its at the same position/rotation as the model was previously.
    // Easier said than done...anims often have wildly different model origins!
    // Ultimately, goal here is to enable continuous seamless animation playback even if model origin changes dramatically between anims.
    if(!animParams.absolute)
    {
        VertexAnimation* anim = animParams.vertexAnimation;
        
        // Sample the hip position/matrix from that animation at frame 0.
        Vector3 hipMeshPos = anim->SampleVertexPosition(0, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
        Matrix4 meshToLocalMatrix = anim->SampleTransformPose(0, mCharConfig->hipAxesMeshIndex).GetMeshToLocalMatrix();
        
        // Convert hip pos to world space. Transform point using mesh->world matrix.
        Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * meshToLocalMatrix).TransformPoint(hipMeshPos);
        
        // The hips are not right at the mesh actor's position. So, calculate offset from hips to mesh actor position.
        // Y-component is zeroed out because we don't want this to change the height, just X/Z plane position.
        Vector3 hipPosToActor = mModelActor->GetPosition() - worldHipPos;
        hipPosToActor.y = 0.0f;
        
        // Set mesh actor to the actor's position.
        // HOWEVER, again, mesh's actual position (represented by hip pos) may be offset (by quite a large amount) from the mesh actor's position!
        // So that's what we apply the hipToActor offset.
        mModelActor->SetPosition(GetPosition() + hipPosToActor);
        
        // Update rotation.
        mModelActor->SetRotation(GetRotation() * mMeshLocalRotation);
    }
    
    mWalkerDOR->SetPosition(mModelActor->GetPosition());
    mWalkerDOR->SetRotation(mModelActor->GetRotation());
    
    // In GK3, a "move" anim is one that is allowed to move the actor. This is like "root motion" in modern engines.
    // When "move" anim ends, the actor/mesh stay where they are. For "non-move" anims, actor/mesh revert to initial pos/rot.
    // Interestingly, the actor still moves with the model during non-move anims...it just reverts at the end.
    mVertexAnimAllowMove = animParams.allowMove;
    if(!mVertexAnimAllowMove)
    {
        // Save start pos/rot for the actor, so it can be reverted.
        mStartVertexAnimPosition = GetPosition();
        mStartVertexAnimRotation = GetRotation();
        
        // Save start pos/rot for the mesh, so it can be reverted.
        mStartVertexAnimMeshPos = GetMeshPosition();
        mStartVertexAnimMeshRotation = GetMeshRotation();
    }
    
    // Save last mesh pos/rot to establish new baseline for "actor follow mesh" function.
    mLastMeshPos = mStartVertexAnimMeshPos;
    mLastMeshRotation = mStartVertexAnimMeshRotation;
}

void GKActor::OnVertexAnimationStop()
{
    // On anim stop, if vertex anim is not allowed to move actor position,
    // we must revert actor back to position when anim started.
    if(!mVertexAnimAllowMove)
    {
        // Move actor back to start position/rotation.
        SetPosition(mStartVertexAnimPosition);
        SetRotation(mStartVertexAnimRotation);
        
        // Move mesh back to starting position/rotation.
        // These are a bit harder because mesh's position relative to mesh actor origin may vary wildly depending on animation being played.
        // To solve that, we'll move back using relative diffs, rather than absolute values.
        mModelActor->GetTransform()->Translate(mStartVertexAnimMeshPos - GetMeshPosition());
        Quaternion diff = Quaternion::Diff(mStartVertexAnimMeshRotation, GetMeshRotation());
        mModelActor->GetTransform()->RotateAround(GetPosition(), diff);
    }
    
    // Because the mesh may have moved back to start pos (if not a move anim),
    // reset the last mesh position/rotation.
    mLastMeshPos = GetMeshPosition();
    mLastMeshRotation = GetMeshRotation();
    
    // Position DOR at model.
    mWalkerDOR->SetPosition(mModelActor->GetPosition());
    mWalkerDOR->SetRotation(mModelActor->GetRotation());
}

Vector3 GKActor::GetMeshPosition()
{
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);
    worldHipPos.y = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(Vector3::Zero).y;
    return worldHipPos;
}

Quaternion GKActor::GetMeshRotation()
{
    Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
    return hipMeshToWorldMatrix.GetRotation();
}
