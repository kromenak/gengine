//
// GKActor.cpp
//
// Clark Kromenaker
//
#include "GKActor.h"

#include "Animator.h"
#include "AnimationNodes.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
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

void GKActor::Init(const Timeblock& timeblock)
{
    // Figure out what clothes to apply, if any.
    Animation* clothesAnim = nullptr;
    for(int i = 0; i < CharacterConfig::kMaxClothesAnims; ++i)
    {
        // If any entry is not set, subsequent entries will also not be set.
        const std::pair<std::string, Animation*>& clothes = mCharConfig->clothesAnims[i];
        if(clothes.first.empty()) { break; }

        // If timeblock string is "Default", then this is the default clothes to wear.
        if(StringUtil::EqualsIgnoreCase(clothes.first, "Default"))
        {
            if(clothesAnim == nullptr)
            {
                clothesAnim = clothes.second;
            }
        }
        else
        {
            // If not "Default", the string must represent a timeblock. Convert it to one.
            Timeblock clothesTimeblock(clothes.first);

            // We use these clothes if our timeblock is at or after the clothes' timeblock.
            // For example, if a character specifies clothes for 110a, and it is 102p, we use the 110a clothes.
            if(timeblock >= clothesTimeblock)
            {
                clothesAnim = clothes.second;
            }
        }
    }

    // Apply the clothes!
    if(clothesAnim != nullptr)
    {
        GEngine::Instance()->GetScene()->GetAnimator()->Start(clothesAnim);
    }
}

void GKActor::StartFidget(FidgetType type)
{
    // Treat "Start None" as "Stop".
    if(type == FidgetType::None)
    {
        StopFidget();
        return;
    }

    // If this is already the active fidget, ignore the start request.
    if(type == mActiveFidget)
    {
        return;
    }

    // Determine which fidget to start.
    // Only *actually* set the fidget and play it if we have a non-null GAS file.
    GAS* newFidget = GetGasForFidget(type);
    if(newFidget != nullptr)
    {
        //std::cout << GetNoun() << ": starting fidget " << newFidget->GetName() << std::endl;
        mGasPlayer->Play(newFidget);
        mActiveFidget = type;
    }
}

void GKActor::StopFidget(std::function<void()> callback)
{
    //std::cout << GetNoun() << ": stopping all fidgets." << std::endl;
    mGasPlayer->Stop(callback);
    mActiveFidget = FidgetType::None;
}

void GKActor::SetIdleFidget(GAS* fidget)
{
    mIdleFidget = fidget;
    CheckUpdateActiveFidget(FidgetType::Idle);
}

void GKActor::SetTalkFidget(GAS* fidget)
{
    mTalkFidget = fidget;
    CheckUpdateActiveFidget(FidgetType::Talk);
}

void GKActor::SetListenFidget(GAS* fidget)
{
    mListenFidget = fidget;
    CheckUpdateActiveFidget(FidgetType::Listen);
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
    //std::cout << "Set Actor Position; actorPos=" << GetPosition() << ", modelPos=" << mModelActor->GetPosition() << std::endl;

    // If the actor is playing a non-move animation, it will revert to its start position when the animation ends...this will revert the SetPosition call!
    // To resolve this, we must update the saved start position when the actor's position is manually changed.
    mStartVertexAnimPosition = position;

    // Move model to align with new position.
    SyncModelToActor();
}

//TODO: SetRotation?

void GKActor::SetHeading(const Heading& heading)
{
    // Update heading of this actor.
    GKObject::SetHeading(heading);

    // If actor is playing a non-move animation, it will revert to its start rotation when the animation ends, reverting this call.
    // To resolve this, we must update the saved start rotation when manually setting the heading/rotation.
    mStartVertexAnimRotation = GetRotation();
    
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
    //std::cout << GetNoun() << ": playing animation " << animParams.vertexAnimation->GetName() << std::endl;

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
    }
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
    diff.IsolateY();
    mModelActor->GetTransform()->RotateAround(GetPosition(), diff);
    
    // Save new baseline model position/rotation.
    mLastModelPosition = GetModelPosition();
    mLastModelRotation = GetModelRotation();

    //std::cout << "Sync Model to Actor; actorPos=" << GetPosition() << ", modelPos=" << mModelActor->GetPosition() << std::endl;
}

void GKActor::SyncActorToModel()
{
    // See how much mesh has moved and translate actor to match.
    Vector3 meshPosition = GetModelPosition();
    GetTransform()->Translate(meshPosition - mLastModelPosition);
    //Debug::DrawLine(mLastModelPosition, meshPosition, Color32::Magenta);
    
    // See how much mesh has rotated and translate actor to match.
    Quaternion meshRotation = GetModelRotation();
    Quaternion diff = Quaternion::Diff(meshRotation, mLastModelRotation);
    diff.IsolateY();
    GetTransform()->Rotate(diff);
    
    // Save new baseline model position/rotation.
    mLastModelPosition = meshPosition;
    mLastModelRotation = meshRotation;

    //std::cout << "Sync Actor to Model; actorPos=" << GetPosition() << ", modelPos=" << mModelActor->GetPosition() << std::endl;
}

GAS* GKActor::GetGasForFidget(FidgetType type)
{
    GAS* gas = nullptr;
    switch(type)
    {
    case FidgetType::None:
        break;
    case FidgetType::Idle:
        gas = mIdleFidget;
        break;
    case FidgetType::Listen:
        gas = mListenFidget;
        break;
    case FidgetType::Talk:
        gas = mTalkFidget;
    }
    return gas;
}

void GKActor::CheckUpdateActiveFidget(FidgetType changedType)
{
    // If we just changed the active fidget...
    if(mActiveFidget == changedType)
    {
        GAS* fidget = GetGasForFidget(changedType);

        // If the gas file for the active fidget has changed to null, that forces fidget to stop.
        // Otherwise, we just immediately play the new gas file.
        if(fidget == nullptr)
        {
            StopFidget();
        }
        else
        {
            mGasPlayer->Play(fidget);
        }
    }
}