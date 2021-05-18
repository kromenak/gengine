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
	
	// GOAL: walk the actor to the initial position/rotation of this anim.
	// Retrieve vertex animation from animation that matches this actor's model.
	// If no vertex anim exists, we can't walk to animation start! This is probably a developer error.
	VertexAnimation* vertexAnim = anim->GetVertexAnimationOnFrameForModel(0, GetModelName());
	if(vertexAnim == nullptr) { return; }
	
    // Sample position on first frame of animation - that's our walk pos.
	Vector3 walkPos = vertexAnim->SampleVertexPosition(0, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
	VertexAnimationTransformPose transformPose = vertexAnim->SampleTransformPose(0, mCharConfig->hipAxesMeshIndex);
	walkPos = transformPose.mMeshToLocalMatrix.TransformPoint(walkPos);
	walkPos.y = GetPosition().y;
        
    // Calculate rotation on first frame of animation - that's our heading.
	Heading heading = Heading::FromQuaternion(transformPose.mMeshToLocalMatrix.GetRotation() * mMeshLocalRotation);
    
	// Walk to that position/heading.
	mWalker->WalkTo(walkPos, heading, walkerBoundary, finishCallback);
    
    // To visualize walk position/heading.
    //Debug::DrawLine(walkPos, walkPos + Vector3::UnitY * 10.0f, Color32::Orange, 10.0f);
    //Debug::DrawLine(walkPos, walkPos + heading.ToVector() * 10.0f, Color32::Red, 10.0f);
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

void GKActor::SetWalkerDOR(GKProp *walkerDOR)
{
    mWalkerDOR = walkerDOR;
}

void GKActor::SnapToFloor()
{
    mWalker->SnapToFloor();
}

Vector3 GKActor::GetHeadPosition() const
{
	// Just use height from floor to approximate head position.
    //TODO: Could probably use head mesh index data from character config? Might be a bit more accurate?
	Vector3 position = GetPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

void GKActor::OnUpdate(float deltaTime)
{
    GKProp::OnUpdate(deltaTime);
    
    // Track mesh movement since last frame and translate actor to match.
    Vector3 meshPosition = GetMeshPosition();
    Vector3 meshPositionChange = meshPosition - mLastMeshPos;
    GetTransform()->Translate(meshPositionChange);
    mLastMeshPos = meshPosition;
    
    // Same idea for rotation. But ONLY track/adjust y-axis change (to avoid actors rotating weirdly).
    Quaternion meshRotation = GetMeshRotation();
    Quaternion meshRotationChange = Quaternion::Diff(meshRotation, mLastMeshRotation);
    meshRotationChange.x = 0.0f;
    meshRotationChange.z = 0.0f;
    meshRotationChange.Normalize();
    GetTransform()->Rotate(meshRotationChange);
    mLastMeshRotation = meshRotation;
    
    // Put heading model at same spot as model actor.
    mWalkerDOR->SetPosition(mModelActor->GetPosition());
    mWalkerDOR->SetRotation(mModelActor->GetRotation());
    
	// Stay on the ground.
	SnapToFloor();
    
    // Also keep mesh on ground (maybe?)
    //TODO: Investigate this a bit more...
    Vector3 modelPos = mModelActor->GetPosition();
    modelPos.y = GetPosition().y;
    mModelActor->SetPosition(modelPos);
}

void GKActor::OnVertexAnimationStart(const VertexAnimParams& animParams)
{
    //std::cout << "VertexAnimStart: " << animParams.vertexAnimation->GetName() << std::endl;
    
    // For relative anims, set model's position/rotation equal to actor's position/rotation.
    if(!animParams.absolute)
    {
        // Update rotation.
        //TODO: This mayyy be incorrect if GetMeshRotation != mModelActor->GetRotation.
        mModelActor->SetRotation(GetRotation() * mMeshLocalRotation);
                
        // Move mesh to actor position.
        // However, the mesh itself may not be exactly at the model actor's origin, so need to calc that diff.
        // Y-component is zeroed out because we don't want this to change the height, just X/Z plane position.
        Vector3 worldMeshPos = GetMeshPosition();
        Vector3 meshToActor = mModelActor->GetPosition() - worldMeshPos;
        meshToActor.y = 0.0f;
        mModelActor->SetPosition(GetPosition() + meshToActor);
    }

    // Put heading model at same spot as model actor.
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
    mLastMeshPos = GetMeshPosition();
    mLastMeshRotation = GetMeshRotation();
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
        mModelActor->GetTransform()->RotateAround(GetPosition(), Quaternion::Diff(mStartVertexAnimMeshRotation, GetMeshRotation()));
    }
    
    // Position DOR at model.
    mWalkerDOR->SetPosition(mModelActor->GetPosition());
    mWalkerDOR->SetRotation(mModelActor->GetRotation());
    
    // Because the mesh may have moved back to start pos (if not a move anim), reset the last mesh position/rotation.
    mLastMeshPos = GetMeshPosition();
    mLastMeshRotation = GetMeshRotation();
}

Vector3 GKActor::GetMeshPosition()
{
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);
    
    //TODO: Is this correct?
    worldHipPos.y = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(Vector3::Zero).y;
    return worldHipPos;
}

Quaternion GKActor::GetMeshRotation()
{
    Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
    return hipMeshToWorldMatrix.GetRotation();
}
