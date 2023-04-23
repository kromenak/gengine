#include "GKActor.h"

#include "Animation.h"
#include "Animator.h"
#include "AnimationNodes.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GAS.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "VertexAnimation.h"
#include "VertexAnimator.h"
#include "Walker.h"

GKActor::GKActor()
{
    // Because of how animation movements and offsets work in GK3, a separate Actor is required for the 3D model.
    mModelActor = new Actor();
    mMeshRenderer = mModelActor->AddComponent<MeshRenderer>();

    // Actors use lit shader.
    mMeshRenderer->SetShader(Services::GetAssets()->LoadShader("3D-Tex-Lit"));

    // Create animation player on the same object as the mesh renderer.
    mVertexAnimator = mModelActor->AddComponent<VertexAnimator>();

    // GasPlayer will go on the actor itself.
    mGasPlayer = AddComponent<GasPlayer>();
}

GKActor::GKActor(Model* model) : GKActor()
{
    // Set actor's 3D model.
    mMeshRenderer->SetModel(model);

    // Use the model's name as the name of the actor.
    // This is sometimes used for gameplay logic, so be careful about changing this!
    SetName(mMeshRenderer->GetModelName());
    
	// Get config for this character.
    // It is assumed that the name of the actor's model corresponds to their three-letter character ID (e.g. GAB, GRA, MOS).
	CharacterConfig& config = Services::Get<CharacterManager>()->GetCharacterConfig(model->GetNameNoExtension());
	mCharConfig = &config;
	
	// Create and configure face controller.
	mFaceController = AddComponent<FaceController>();
	mFaceController->SetCharacterConfig(config);
	
	// Add walker and configure it.
	mWalker = AddComponent<Walker>();
	mWalker->SetCharacterConfig(config);

    // Init the model so it faces the same way as the Actor (in case the model isn't authored with the usual -Z is forward).
    SyncModelToActor();
}

GKActor::GKActor(const SceneActor& actorDef) : GKActor(actorDef.model)
{
    // Set noun (GABRIEL, GRACE, etc) used to refer to Actor in Noun/Verb/Case logic.
    SetNoun(actorDef.noun);

    // Set actor's initial position and rotation.
    if(!actorDef.positionName.empty())
    {
        const ScenePosition* scenePos = GEngine::Instance()->GetScene()->GetPosition(actorDef.positionName);
        if(scenePos != nullptr)
        {
            SetPosition(scenePos->position);
            SetHeading(scenePos->heading);
        }
        else
        {
            std::cout << "Invalid position for actor: " << actorDef.positionName << std::endl;
        }
    }

    // Save actor's GAS references.
    SetIdleFidget(actorDef.idleGas);
    SetTalkFidget(actorDef.talkGas);
    SetListenFidget(actorDef.listenGas);

    // If it should be hidden by default, hide it.
    SetActive(!actorDef.hidden);
}

void GKActor::Init(const SceneData& sceneData)
{
    // Start in idle state.
    StartFidget(GKActor::FidgetType::Idle);

    // Tell actor to use this scene's walker boundary.
    mWalker->SetWalkerBoundary(sceneData.GetWalkerBoundary());

    // Figure out what clothes anim to apply, if any.
    const Timeblock& timeblock = sceneData.GetTimeblock();
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
    if(clothesAnim != nullptr)
    {
        GEngine::Instance()->GetScene()->GetAnimator()->Start(clothesAnim);
    }

    // Apply lighting settings from scene.
    for(Material& material : mMeshRenderer->GetMaterials())
    {
        material.SetVector4("uLightPos", Vector4(sceneData.GetGlobalLightPosition(), 1.0f));
        //TODO: Color?
        //TODO: Ambient Color?
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
        //printf("%s: starting fidget %s\n", GetNoun().c_str(), newFidget->GetName().c_str());
        mGasPlayer->Play(newFidget);
        mActiveFidget = type;
    }
}

void GKActor::StopFidget(std::function<void()> callback)
{
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

void GKActor::WalkTo(const Vector3& position, std::function<void()> finishCallback)
{
    mWalker->WalkTo(position, finishCallback);
}

void GKActor::WalkTo(const Vector3& position, const Heading& heading, std::function<void()> finishCallback)
{
	mWalker->WalkTo(position, heading, finishCallback);
}

void GKActor::WalkToGas(const Vector3& position, const Heading& heading, std::function<void()> finishCallback)
{
    // This version of the function is needed just to tell the walker if this walk request is coming from a GAS script or not!
    mWalker->WalkToGas(position, heading, finishCallback);
}

void GKActor::WalkToSee(GKObject* target, std::function<void()> finishCallback)
{
    mWalker->WalkToSee(target, finishCallback);
}

void GKActor::WalkToAnimationStart(Animation* anim, std::function<void()> finishCallback)
{
	// Need a valid anim.
	if(anim == nullptr) { return; }
	
	// GOAL: walk the actor to the initial position/rotation of this anim.
	// Retrieve vertex animation that matches this actor's model.
	// If no vertex anim exists, we can't walk to animation start! This is probably a developer error.
    VertexAnimNode* vertexAnimNode = anim->GetVertexAnimationOnFrameForModel(0, mMeshRenderer->GetModelName());
	if(vertexAnimNode == nullptr) { return; }

    // Sample position/pose on first frame of animation.
    // That gives our walk pos/rotation, BUT it is in the actor's local space.
    Vector3 walkPos = vertexAnimNode->vertexAnimation->SampleVertexPosition(0, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
    VertexAnimationTransformPose transformPose = vertexAnimNode->vertexAnimation->SampleTransformPose(0, mCharConfig->hipAxesMeshIndex);

    // If this is an absolute animation, the above position needs to be converted to world space.
    Matrix4 transformMatrix = transformPose.meshToLocalMatrix;
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

Vector3 GKActor::GetWalkDestination() const
{
	if(!mWalker->IsWalking()) { return GetPosition(); }
	return mWalker->GetDestination();
}

void GKActor::SnapToFloor()
{
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene != nullptr)
    {
        Vector3 pos = GetPosition();
        pos.y = scene->GetFloorY(pos);
        SetPosition(pos);
    }
}

const std::string& GKActor::GetShoeType() const
{
    if(mCharConfig != nullptr)
    {
        return mCharConfig->shoeType;
    }
    else
    {
        static std::string defaultShoeType = "Male Leather";
        return defaultShoeType;
    }
}

Vector3 GKActor::GetHeadPosition() const
{
    // Get center point of head mesh.
    Mesh* headMesh = mMeshRenderer->GetMesh(mCharConfig->headMeshIndex);
    if(headMesh != nullptr)
    {
        Vector3 meshHeadPos = headMesh->GetAABB().GetCenter();
        Vector3 worldHeadPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * headMesh->GetMeshToLocalMatrix()).TransformPoint(meshHeadPos);
        return worldHeadPos;
    }

    // If head mesh isn't present for some reason, approximate based on position and height.
	Vector3 position = GetPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

void GKActor::SetPosition(const Vector3& position)
{
    // Move actor to position using base class behavior.
    Actor::SetPosition(position);

    // If the actor is playing a non-move animation, it will revert to its start position when the animation ends...this will revert the SetPosition call!
    // To resolve this, we must update the saved start position when the actor's position is manually changed.
    mStartVertexAnimPosition = position;

    // Move model to align with new position.
    SyncModelToActor();
}

void GKActor::Rotate(float angle)
{
    // Rotate this actor by the angle.
    GetTransform()->Rotate(Vector3::UnitY, angle);

    // As with SetHeading, we must update this value to avoid reverting rotations after current anim completes.
    mStartVertexAnimRotation = GetRotation();

    // Make sure model matches this rotation.
    SyncModelToActor();
}

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

void GKActor::StartAnimation(VertexAnimParams& animParams)
{
    // Don't let a GAS anim override a non-GAS anim.
    if(animParams.fromAutoScript && mVertexAnimator->IsPlayingNotAutoscript()) { return; }

    // If this is not a GAS anim, pause any running GAS.
    if(!animParams.fromAutoScript)
    {
        StopFidget();
    }

    // Set anim stop callback.
    animParams.stopCallback = std::bind(&GKActor::OnVertexAnimationStop, this);

    // Start the animation.
    // Note that this will sample the first frame of the animation, updating the model's positions/rotations.
    mVertexAnimator->Start(animParams);

    // For absolute anims, position model exactly as specified.
    if(animParams.absolute)
    {
        mModelActor->SetPosition(animParams.absolutePosition);
        mModelActor->SetRotation(animParams.absoluteHeading.ToQuaternion());
    }
    
    // Relative anims play with the 3D model starting at the Actor's current position/rotation.
    // So, do a sync to ensure the 3D model is at the Actor's position.
    //TODO: Should this occur before we call VertexAnimator::Start?
    if(!animParams.absolute)
    {
        SyncModelToActor();
    }

    // In GK3, a "move" anim keeps the model & actor at their final position/rotation when the animation reaches its end.
    // Non-move anims revert the model/actor back to their start positions/rotations when the animation ends.
    // So, if this is NOT a move animation, store current position/rotation so we can revert at the end.
    mVertexAnimAllowMove = animParams.allowMove;
    if(!mVertexAnimAllowMove)
    {
        // Save start pos/rot for the actor, so it can be reverted.
        mStartVertexAnimPosition = GetPosition();
        mStartVertexAnimRotation = GetRotation();
    }
}

void GKActor::SampleAnimation(VertexAnimParams& animParams, int frame)
{
    // Sample the animation on the desired frame.
    mVertexAnimator->Sample(animParams.vertexAnimation, frame);

    // For absolute anims, position model exactly as specified.
    if(animParams.absolute)
    {
        mModelActor->SetPosition(animParams.absolutePosition);
        mModelActor->SetRotation(animParams.absoluteHeading.ToQuaternion());
    }

    // For relative anims, move model to match actor's position/rotation.
    if(!animParams.absolute)
    {
        SyncModelToActor();
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

void GKActor::StopAnimation(VertexAnimation* anim)
{
    // NOTE: passing nullptr will stop ALL playing animations.
    mVertexAnimator->Stop(anim);
}

AABB GKActor::GetAABB()
{
    return mMeshRenderer->GetAABB();
}

void GKActor::OnActive()
{
    // My model becomes active when I become active.
    mModelActor->SetActive(true);
}

void GKActor::OnInactive()
{
    // My model becomes inactive when I become inactive.
    mModelActor->SetActive(false);
}

void GKActor::OnUpdate(float deltaTime)
{
    // Have actor follow model movement.
    SyncActorToModel();

    // OPTIONAL: draw axes to visualize Actor position and facing.
    //Debug::DrawAxes(GetTransform()->GetLocalToWorldMatrix());
}

void GKActor::OnVertexAnimationStop()
{
    // If this wasn't a "move" anim, we must revert back to the initial position/rotation on animation end.
    if(!mVertexAnimAllowMove)
    {
        // Move actor back to start position/rotation.
        Actor::SetPosition(mStartVertexAnimPosition);
        Actor::SetRotation(mStartVertexAnimRotation);

        // Move model to match actor.
        SyncModelToActor();
    }
}

Vector3 GKActor::GetModelPosition()
{
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);

    // Get left/right shoe positions.
    Vector3 meshLeftShoePos = mMeshRenderer->GetMesh(mCharConfig->leftShoeAxesMeshIndex)->GetSubmesh(mCharConfig->leftShoeAxesGroupIndex)->GetVertexPosition(mCharConfig->leftShoeAxesPointIndex);
    Vector3 worldLeftShoePos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->leftShoeAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshLeftShoePos);

    Vector3 meshRightShoePos = mMeshRenderer->GetMesh(mCharConfig->rightShoeAxesMeshIndex)->GetSubmesh(mCharConfig->rightShoeAxesGroupIndex)->GetVertexPosition(mCharConfig->rightShoeAxesPointIndex);
    Vector3 worldRightShoePos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->rightShoeAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshRightShoePos);

    //Debug::DrawLine(worldHipPos, worldLeftShoePos, Color32::Cyan);
    //Debug::DrawLine(worldHipPos, worldRightShoePos, Color32::Red);
    //Debug::DrawLine(worldLeftShoePos, worldRightShoePos, Color32::Magenta);
    
    // Use left/right shoe positions to determine our floor position.
    // We're assuming that the actor's lowest foot, minus show thickness, is the floor height.
    Vector3 floorPos = worldHipPos;
    floorPos.y = Math::Min(worldLeftShoePos.y, worldRightShoePos.y);
    floorPos.y -= mCharConfig->shoeThickness;
    
    // EXPERIMENT: Trying to obtain a consistent forward based on shoes? Not sure if this works.
    //Vector3 leftShoeToHips = worldHipPos - worldLeftShoePos;
    //Vector3 rightShoeToHips = worldHipPos - worldRightShoePos;
    //Vector3 modelForward = Vector3::Normalize(Vector3::Cross(rightShoeToHips, leftShoeToHips));
    //Debug::DrawLine(floorPos, floorPos + modelForward * 5, Color32::Magenta);

    return floorPos;
}

Quaternion GKActor::GetModelRotation()
{
    // Note that this is the rotation of the model actor, so it is facing in opposite direction of actor.
    Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
    return hipMeshToWorldMatrix.GetRotation();
}

void GKActor::SyncModelToActor()
{
    // MOVE MODEL TO ACTOR POSITION
    {
        // Here's a tricky thing: the 3D model's *visual* position IS NOT always identical to the 3D model's actor position!
        // The 3D model vertices may be significantly offset in the local space of the 3D model actor.
        // To account for that, calculate that offset of the 3D model in local space.
        Vector3 modelVisualOffset = mModelActor->GetPosition() - GetModelPosition();
        
        // Move the model actor to our position, taking this offset into account.
        // This causes the model's visuals to be exactly at our position.
        mModelActor->SetPosition(GetPosition() + modelVisualOffset);
    }

    // ROTATE MODEL TO ACTOR ROTATION
    {
        // We want the model to face the same direction as us.
        // Remember, 3D character models are *backwards* (forward is down the negative Z axis), so the desired rotation is our rotation + 180 degrees.
        Quaternion desiredRotation = GetRotation() * Quaternion(Vector3::UnitY, Math::kPi);

        // Get the 3D model's current *visual* rotation (which may differ from the 3D model's actor's rotation).
        Quaternion currentRotation = GetModelRotation();

        // Get the diff, but only about the Y-axis, to go from models current rotation to the desired rotation.
        Quaternion diff = Quaternion::Diff(desiredRotation, currentRotation);
        diff.IsolateY();

        // Apply that rotation so the model is now visually facing the same direction as us.
        mModelActor->GetTransform()->RotateAround(GetPosition(), diff);
    }

    // Model's position and rotation have changed, so save updated "last known" values.
    mLastModelPosition = GetModelPosition();
    mLastModelRotation = GetModelRotation();
}

void GKActor::SyncActorToModel()
{
    // MOVE OUR POSITION TO MODEL POSITION
    Vector3 modelPosition = GetModelPosition();
    {
        // Fortunately, this is pretty easy: get the model's visual position, put our position there.
        GetTransform()->SetPosition(modelPosition);
    }

    // MOVE OUR ROTATION TO MODEL ROTATION
    Quaternion modelRotation = GetModelRotation();
    {
        // Unfortunately, we can't just set this directly b/c the rotation calculated does not always visually align with what forward should be.
        // For example, the hip forward during most animations match the visuals, but for the "turn before walk" anim, it doesn't (was that an authoring error?).
        // In other words, either my calculation is wrong, or the models were not authored consistently.
        
        // To get around this, we can try to utilize a "diff" approach.
        // We know that if the model's rotation changes, the diff represents an amount of rotation that occurred visually.
        // So, we can calculate that diff and apply an equal rotation to ourselves to (hopefully) keep our rotation correct.

        // Get diff from last time we checked, only about Y-axis.
        Quaternion diff = Quaternion::Diff(modelRotation, mLastModelRotation);
        diff.IsolateY();

        // Rotate ourselves by that amount.
        GetTransform()->Rotate(diff);
    }
    
    // Save new baseline model position/rotation.
    mLastModelPosition = modelPosition;
    mLastModelRotation = modelRotation;
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
        break;
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
