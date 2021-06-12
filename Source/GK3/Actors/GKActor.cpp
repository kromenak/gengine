//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include "AnimationNodes.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Services.h"
#include "VertexAnimator.h"
#include "Walker.h"

GKActor::GKActor(Model* model) : GKProp(true)
{
    // Set actor's 3D model.
    mModelRenderer->SetModel(model);
    
    // Use lit shader.
    std::vector<Material>& materials = mModelRenderer->GetMaterials();
    for(Material& material : materials)
    {
        material.SetShader(Services::GetAssets()->LoadShader("3D-Tex-Lit"));
    }
    
	// Get config for this character.
	CharacterConfig& config = Services::Get<CharacterManager>()->GetCharacterConfig(GetModelName());
	mCharConfig = &config;
	
	// Create and configure face controller.
	mFaceController = AddComponent<FaceController>();
	mFaceController->SetCharacterConfig(config);
	
	// Add walker and configure it.
	mWalker = AddComponent<Walker>();
	mWalker->SetCharacterConfig(config);
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
	mWalker->WalkTo(GetPosition(), heading, finishCallback);
}

void GKActor::WalkTo(const Vector3& position, const Heading& heading, std::function<void()> finishCallback)
{
	mWalker->WalkTo(position, heading, finishCallback);
}

void GKActor::WalkTo(const Vector3& position, std::function<void()> finishCallback)
{
	mWalker->WalkTo(position, finishCallback);
}

void GKActor::WalkToAnimationStart(Animation* anim, std::function<void()> finishCallback)
{
	// Need a valid anim.
	if(anim == nullptr) { return; }
	
	// GOAL: walk the actor to the initial position/rotation of this anim.
	// Retrieve vertex animation from animation that matches this actor's model.
	// If no vertex anim exists, we can't walk to animation start! This is probably a developer error.
    VertexAnimNode* vertexAnimNode = anim->GetVertexAnimationOnFrameForModel(0, GetModelName());
	if(vertexAnimNode == nullptr) { return; }

    // Sample position/pose on first frame of animation.
    // That gives our walk pos/rotation, BUT it is in the actor's local space.
    Vector3 walkPos = vertexAnimNode->vertexAnimation->SampleVertexPosition(0, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
    VertexAnimationTransformPose transformPose = vertexAnimNode->vertexAnimation->SampleTransformPose(0, mCharConfig->hipAxesMeshIndex);

    // If this is an absolute animation, the above position needs to be further converted to world space.
    Matrix4 transformMatrix = transformPose.mMeshToLocalMatrix;
    if(vertexAnimNode->absolute)
    {
        Vector3 animWorldPos = vertexAnimNode->CalcAbsolutePosition();
        Quaternion modelToActorRot(Vector3::UnitY, Math::ToRadians(vertexAnimNode->absoluteWorldToModelHeading));

        Matrix4 localToWorldMatrix = Matrix4::MakeTranslate(animWorldPos) * Matrix4::MakeRotate(modelToActorRot);
        transformMatrix = localToWorldMatrix * transformMatrix;
    }

    // Calculate walk pos in world space.
    walkPos = transformMatrix.TransformPoint(walkPos);
    walkPos.y = GetPosition().y;

    // Calculate rotation on first frame of animation - that's our heading.
    Heading heading = Heading::FromQuaternion(transformMatrix.GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));

    // Walk to that position/heading.
    mWalker->WalkTo(walkPos, heading, finishCallback);

    // To visualize walk position/heading.
    //Debug::DrawLine(walkPos, walkPos + Vector3::UnitY * 10.0f, Color32::Orange, 10.0f);
    //Debug::DrawLine(walkPos, walkPos + heading.ToVector() * 10.0f, Color32::Red, 10.0f);
}

void GKActor::WalkToSee(const std::string& targetName, const Vector3& targetPosition, std::function<void()> finishCallback)
{
	mWalker->WalkToSee(targetName, targetPosition, finishCallback);
}

void GKActor::SetWalkerBoundary(WalkerBoundary* walkerBoundary)
{
    mWalker->SetWalkerBoundary(walkerBoundary);
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
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene != nullptr)
    {
        Vector3 pos = GetPosition();
        pos.y = scene->GetFloorY(pos) + mCharConfig->shoeThickness;
        SetPosition(pos);
    }
}

Vector3 GKActor::GetHeadPosition() const
{
	// Just use height from floor to approximate head position.
    //TODO: Could probably use head mesh index data from character config? Might be a bit more accurate?
	Vector3 position = GetPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

void GKActor::SetPosition(const Vector3& position)
{
    // Move actor to position.
    Actor::SetPosition(position);
    
    // Move model to align with new position.
    SyncModelToActor();
}

//TODO: SetRotation?

void GKActor::SetHeading(const Heading& heading)
{
    // Update heading of this actor.
    GKObject::SetHeading(heading);
    
    // Update model to match.
    SyncModelToActor();
}

void GKActor::OnUpdate(float deltaTime)
{
    GKProp::OnUpdate(deltaTime);
    
    // Have actor follow model movement.
    SyncActorToModel();
    
    // Put heading model at same spot as model actor.
    if(mWalkerDOR != nullptr)
    {
        mWalkerDOR->SetPosition(mModelActor->GetPosition());
        mWalkerDOR->SetRotation(mModelActor->GetRotation());
    }
}

void GKActor::OnVertexAnimationStart(const VertexAnimParams& animParams)
{
    // For relative anims, move model to match actor's position/rotation.
    if(!animParams.absolute)
    {
        SyncModelToActor();
    }

    // Put heading model at same spot as model actor.
    if(mWalkerDOR != nullptr)
    {
        mWalkerDOR->SetPosition(mModelActor->GetPosition());
        mWalkerDOR->SetRotation(mModelActor->GetRotation());
    }
     
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
        //mStartVertexAnimModelPosition = GetModelPosition();
        //mStartVertexAnimModelRotation = GetModelRotation();
    }
    
    // Save last mesh pos/rot to establish new baseline for "actor follow mesh" function.
    //mLastModelPosition = GetModelPosition();
    //mLastModelRotation = GetModelRotation();
}

void GKActor::OnVertexAnimationStop()
{
    // On anim stop, if vertex anim is not allowed to move actor position,
    // we must revert actor back to position when anim started.
    if(!mVertexAnimAllowMove)
    {
        // Move actor back to start position/rotation.
        Actor::SetPosition(mStartVertexAnimPosition);
        SetRotation(mStartVertexAnimRotation);
        
        // Move model to match actor.
        SyncModelToActor();
        
        // Move mesh back to starting position/rotation.
        // These are a bit harder because mesh's position relative to mesh actor origin may vary wildly depending on animation being played.
        // To solve that, we'll move back using relative diffs, rather than absolute values.
        //mModelActor->GetTransform()->Translate(mStartVertexAnimMeshPos - GetModelPosition());
        //mModelActor->GetTransform()->RotateAround(GetPosition(), Quaternion::Diff(mStartVertexAnimMeshRotation, GetModelRotation()));
    
        // Because the mesh may have moved back to start pos (if not a move anim), reset the last mesh position/rotation.
        //mLastMeshPos = GetModelPosition();
        //mLastMeshRotation = GetModelRotation();
    }
    
    // Position DOR at model.
    if(mWalkerDOR != nullptr)
    {
        mWalkerDOR->SetPosition(mModelActor->GetPosition());
        mWalkerDOR->SetRotation(mModelActor->GetRotation());
    }
}

Vector3 GKActor::GetModelPosition()
{
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);
    
    // Hips are not at floor level, but we want position at floor level.
    worldHipPos.y = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(Vector3::Zero).y;
    return worldHipPos;
}

Quaternion GKActor::GetModelRotation()
{
    // Note that this is the rotation of the model actor, so it is facing in opposite direction of actor.
    Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mModelRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
    return hipMeshToWorldMatrix.GetRotation();
}

void GKActor::SyncModelToActor()
{
    // Move model to actor's position.
    Vector3 modelOffset = mModelActor->GetPosition() - GetModelPosition();
    //modelOffset.y = 0.0f; // zeroed out to change just X/Z plane position.
    mModelActor->SetPosition(GetPosition() + modelOffset);
    
    // Rotate model to actor's rotation.
    Quaternion desiredRotation = GetRotation() * Quaternion(Vector3::UnitY, Math::kPi);
    Quaternion currentRotation = GetModelRotation();
    Quaternion diff = Quaternion::Diff(desiredRotation, currentRotation);
    diff.x = 0.0f;
    diff.z = 0.0f;
    diff.Normalize();
    mModelActor->GetTransform()->RotateAround(GetPosition(), diff);
    
    // Save new baseline model position/rotation.
    mLastModelPosition = GetModelPosition();
    mLastModelRotation = GetModelRotation();
}

void GKActor::SyncActorToModel()
{
    // See how much mesh has moved and translate actor to match.
    Vector3 meshPosition = GetModelPosition();
    GetTransform()->Translate(meshPosition - mLastModelPosition);
    //Debug::DrawLine(mLastMeshPos, meshPosition, Color32::Magenta, 5.0f);
    
    // See how much mesh has rotated and translate actor to match.
    Quaternion meshRotation = GetModelRotation();
    Quaternion diff = Quaternion::Diff(meshRotation, mLastModelRotation);
    diff.x = 0.0f;
    diff.z = 0.0f;
    diff.Normalize();
    GetTransform()->Rotate(diff);
    
    // Save new baseline model position/rotation.
    mLastModelPosition = meshPosition;
    mLastModelRotation = meshRotation;
}
