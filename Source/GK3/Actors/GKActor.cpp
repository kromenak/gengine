#include "GKActor.h"

#include "Animation.h"
#include "Animator.h"
#include "AnimationNodes.h"
#include "AssetManager.h"
#include "BSP.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "FaceController.h"
#include "GAS.h"
#include "GasPlayer.h"
#include "GKProp.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "SceneManager.h"
#include "StringUtil.h"
#include "VertexAnimation.h"
#include "VertexAnimator.h"
#include "Walker.h"

GKActor::GKActor(const SceneActor* actorDef) :
    mActorDef(actorDef)
{
    // Set noun (GABRIEL, GRACE, etc) used to refer to Actor in Noun/Verb/Case logic.
    SetNoun(mActorDef->noun);

    // Set name to the name of the 3D model, which is a three-letter "character ID" (e.g. GAB, GRA, MOS).
    // This is sometimes used for gameplay logic, so be careful about changing this!
    SetName(mActorDef->model->GetNameNoExtension());

    // Grab this character's config using its three-letter "character ID".
    CharacterConfig& config = gCharacterManager.GetCharacterConfig(GetName());
    mCharConfig = &config;

    // Create a separate Actor for the GKActor's 3D model visuals.
    // This is required b/c the 3D model often moves and is positioned independently of this GKActor instance.
    mModelActor = new Actor();

    // Add 3D model renderer using lit textured shader.
    mMeshRenderer = mModelActor->AddComponent<MeshRenderer>();
    mMeshRenderer->SetModel(mActorDef->model);
    mMeshRenderer->SetShader(gAssetManager.LoadShader("3D-Tex-Lit"));
    
    // Add vertex animator so the 3D model can be animated.
    mVertexAnimator = mModelActor->AddComponent<VertexAnimator>();

    // Add GasPlayer to control logic flow for fidgets.
    mGasPlayer = AddComponent<GasPlayer>();

    // Add a face controller to this actor.
    mFaceController = AddComponent<FaceController>();
    mFaceController->SetCharacterConfig(config);

    // Add a walker to this actor.
    mWalker = AddComponent<Walker>();
    mWalker->SetCharacterConfig(config);
}

void GKActor::Init(const SceneData& sceneData)
{
    // FIRST: sampling the first frame of the "walk start" anim ensures all 3D models are at a sane default.
    // This matches the behavior of the original game when a GKActor is spawned with no position or fidgets.
    gSceneManager.GetScene()->GetAnimator()->Sample(mCharConfig->walkStartAnim, 0);

    // Set actor's initial position and rotation, if any was specified.
    if(!mActorDef->positionName.empty())
    {
        const ScenePosition* scenePos = gSceneManager.GetScene()->GetPosition(mActorDef->positionName);
        if(scenePos != nullptr)
        {
            SetPosition(scenePos->position);
            SetHeading(scenePos->heading);
        }
    }
    
    // Set initial idle/talk/listen fidgets for this actor.
    SetIdleFidget(mActorDef->idleGas);
    SetTalkFidget(mActorDef->talkGas);
    SetListenFidget(mActorDef->listenGas);

    // If we should be hidden by default, set inactive.
    SetActive(!mActorDef->hidden);

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
            // If not "Default", the string must represent an actual timeblock. Convert it to one.
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
        gSceneManager.GetScene()->GetAnimator()->Start(clothesAnim);
    }

    // Apply lighting settings from scene.
    for(Material& material : mMeshRenderer->GetMaterials())
    {
        material.SetVector4("uLightPos", Vector4(sceneData.GetGlobalLightPosition(), 1.0f));
        //TODO: Color?
        //TODO: Ambient Color?
    }

    // Sample an init anim (if any) that poses the GKActor as needed for scene start.
    gSceneManager.GetScene()->GetAnimator()->Sample(mActorDef->initAnim, 0);
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
    Vector3 pos = GetPosition();
    pos.y = mFloorHeight;
    SetPosition(pos);
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
	Vector3 position = GetFloorPosition();
	position.y += mCharConfig->walkerHeight;
	return position;
}

Vector3 GKActor::GetFloorPosition() const
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

    // Use left/right shoe positions to determine our floor position.
    // We're assuming that the actor's lowest foot, minus show thickness, is the floor height.
    Vector3 floorPos = worldHipPos;
    floorPos.y = Math::Min(worldLeftShoePos.y, worldRightShoePos.y);
    floorPos.y -= mCharConfig->shoeThickness;
    return floorPos;
}

void GKActor::SetPosition(const Vector3& position)
{
    // Move actor to position using base class behavior.
    Actor::SetPosition(position);

    // If the actor is playing a non-move animation, it will revert to its start position when the animation ends...this will revert the SetPosition call!
    // To resolve this, we must update the saved start position when the actor's position is manually changed.
    mStartVertexAnimPosition = position;

    // Move model to desired new position.
    SetModelPositionToActorPosition();

    // Update floor info, since actor moved to a new spot.
    RefreshFloorInfo();
}

void GKActor::Rotate(float angle)
{
    // Rotate this actor by the angle.
    GetTransform()->Rotate(Vector3::UnitY, angle);

    // As with SetHeading, we must update this value to avoid reverting rotations after current anim completes.
    mStartVertexAnimRotation = GetRotation();

    // Rotate model to desired new rotation.
    SetModelRotationToActorRotation();
}

void GKActor::SetHeading(const Heading& heading)
{
    // Update heading of this actor.
    GKObject::SetHeading(heading);

    // If actor is playing a non-move animation, it will revert to its start rotation when the animation ends, reverting this call.
    // To resolve this, we must update the saved start rotation when manually setting the heading/rotation.
    mStartVertexAnimRotation = GetRotation();

    // Rotate model to desired new rotation.
    SetModelRotationToActorRotation();
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
    if(!animParams.absolute)
    {
        SetModelPositionToActorPosition();
        SetModelRotationToActorRotation();
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
        SetModelPositionToActorPosition();
        SetModelRotationToActorRotation();
    }

    // In GK3, a "move" anim is one that is allowed to move the actor. This is like "root motion" in modern engines.
    // When "move" anim ends, the actor/mesh stay where they are. For "non-move" anims, actor/mesh revert to initial pos/rot.
    // Interestingly, the actor still moves with the model during non-move anims...it just reverts at the end.
    /*
    mVertexAnimAllowMove = animParams.allowMove;
    if(!mVertexAnimAllowMove)
    {
        // Save start pos/rot for the actor, so it can be reverted.
        mStartVertexAnimPosition = GetPosition();
        mStartVertexAnimRotation = GetRotation();
    }
    */

    //SetModelPositionToActorPosition();
    //SetModelRotationToActorRotation();
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
    // OPTIONAL: draw axes to visualize Actor position and facing.
    //Debug::DrawAxes(GetTransform()->GetLocalToWorldMatrix());

    // Set actor position to match model's floor position.
    GetTransform()->SetPosition(GetFloorPosition());

    // Set actor facing direction to match the model's facing direction.
    {
        // Get mine and model's facing directions.
        Vector3 myFacingDir = GetHeading().ToDirection();
        Vector3 modelFacingDir = GetModelFacingDirection();

        // Calculate angle I should rotate to match model's direction.
        // Use cross product to determine clockwise or counter-clockwise rotation.
        float angleRadians = Math::Acos(Math::Clamp(Vector3::Dot(myFacingDir, modelFacingDir), -1.0f, 1.0f));
        if(Vector3::Cross(myFacingDir, modelFacingDir).y < 0)
        {
            angleRadians *= -1;
        }

        // Rotate desired direction and amount.
        GetTransform()->Rotate(Vector3::UnitY, angleRadians);
    }

    // Update floor info based on updated position.
    RefreshFloorInfo();
}

void GKActor::OnVertexAnimationStop()
{
    // If this wasn't a "move" anim, we must revert back to the initial position/rotation on animation end.
    if(!mVertexAnimAllowMove)
    {
        Actor::SetPosition(mStartVertexAnimPosition);
        Actor::SetRotation(mStartVertexAnimRotation);
    }
}

Vector3 GKActor::GetModelFacingDirection() const
{
    // There are a few different ways to possibly calculate the model's facing direction.
    // The method used may differ from Actor to Actor or be based on the current state of the Actor.
    Vector3 facingDir = Vector3::UnitZ;

    if(mModelFacingHelper != nullptr)
    {
        // The facing helper is an arrow/triangle. Get it's three points in world space.
        Mesh* arrowMesh = mModelFacingHelper->GetMeshRenderer()->GetMesh(0);
        Submesh* arrowSubmesh = arrowMesh->GetSubmesh(0);
        Matrix4 meshToWorldMatrix = mModelFacingHelper->GetTransform()->GetLocalToWorldMatrix() * arrowMesh->GetMeshToLocalMatrix();

        Vector3 worldPoint1 = meshToWorldMatrix.TransformPoint(arrowSubmesh->GetVertexPosition(0));
        Vector3 worldPoint2 = meshToWorldMatrix.TransformPoint(arrowSubmesh->GetVertexPosition(1));
        Vector3 worldPoint3 = meshToWorldMatrix.TransformPoint(arrowSubmesh->GetVertexPosition(2));

        // (Optionally) visualize the facing helper triangle in world space.
        /*
        {
            //Debug::DrawLine(worldPoint1, worldPoint1 + Vector3::UnitY * 10.0f, Color32::Green);
            //Debug::DrawLine(worldPoint2, worldPoint2 + Vector3::UnitY * 10.0f, Color32::Cyan);
            //Debug::DrawLine(worldPoint3, worldPoint3 + Vector3::UnitY * 10.0f, Color32::Magenta);

            Debug::DrawLine(worldPoint1, worldPoint2, Color32::Blue);
            Debug::DrawLine(worldPoint2, worldPoint3, Color32::Blue);
            Debug::DrawLine(worldPoint3, worldPoint1, Color32::Blue);
            Debug::DrawLine(GetPosition(), mModelFacingHelper->GetPosition(), Color32::White);
        }
        */

        // SCENARIO A: We have a model facing helper, and it is currently in use (it's animating or we are walking).
        if(mModelFacingHelper->GetComponent<VertexAnimator>()->IsPlaying() || mWalker->IsWalkingExceptTurn())
        {
            // Use those three points to calculate a facing direction. The "point" of the arrow is pt 1.
            // OF COURSE these points aren't consistent...Mosely uses different ones.
            if(StringUtil::EqualsIgnoreCase(GetName(), "MOS"))
            {
                facingDir = Vector3::Normalize(worldPoint3 - ((worldPoint1 + worldPoint2) / 2));
            }
            else
            {
                facingDir = Vector3::Normalize(worldPoint1 - ((worldPoint2 + worldPoint3) / 2));
            }
        }
        else // SCENARIO B: We have a model facing helper, but not currently in use.
        {
            // Calculate head and shoe positions to create a triangle.
            Vector3 meshHeadPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix().GetTranslation();
            Vector3 worldHeadPos = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(meshHeadPos);

            Vector3 meshLeftShoePos = mMeshRenderer->GetMesh(mCharConfig->leftShoeAxesMeshIndex)->GetMeshToLocalMatrix().GetTranslation();
            Vector3 worldLeftShoePos = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(meshLeftShoePos);

            Vector3 meshRightShoePos = mMeshRenderer->GetMesh(mCharConfig->rightShoeAxesMeshIndex)->GetMeshToLocalMatrix().GetTranslation();
            Vector3 worldRightShoePos = mModelActor->GetTransform()->GetLocalToWorldMatrix().TransformPoint(meshRightShoePos);

            // Imagine these three points are a triangle. Calculate the surface normal of the triangle. This is our facing direction.
            Vector3 vec1 = worldRightShoePos - worldLeftShoePos;
            Vector3 vec2 = worldHeadPos - worldLeftShoePos;
            Vector3 perp = Vector3::Cross(vec1, vec2);
            perp.y = 0.0f;
            facingDir = Vector3::Normalize(perp);

            // (Optionally) visualize this method.
            /*
            {
                Debug::DrawLine(worldRightShoePos, worldLeftShoePos, Color32::Magenta, 0.0f);
                Debug::DrawLine(worldHeadPos, worldRightShoePos, Color32::Magenta, 0.0f);
                Debug::DrawLine(worldHeadPos, worldLeftShoePos, Color32::Magenta, 0.0f);
            }
            */

            // If we use this approach, be sure to keep the facing helper synchronized.
            mModelFacingHelper->SetPosition(mModelActor->GetPosition());
            mModelFacingHelper->SetRotation(mModelActor->GetRotation());
        }
    }
    else // SCENARIO C: No facing helper, this is a very simple Actor (e.g. Chicken).
    {
        // Get the hip axis, convert y-axis to a facing direction.
        // Remember, models are facing down negative z-axis, so need to negate the axis we get back here.
        Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
        facingDir = -hipMeshToWorldMatrix.GetYAxis();
    }

    //Debug::DrawLine(GetPosition(), GetPosition() + facingDir * 10.0f, Color32::Blue);
    return facingDir;
}

void GKActor::SetModelPositionToActorPosition()
{
    // Here's a tricky thing: the 3D model's *visual* position IS NOT always identical to the 3D model's *actor* position!
    // The 3D model vertices may be significantly offset in the local space of the 3D model actor.
    // To account for that, calculate the offset.
    Vector3 modelVisualOffset = mModelActor->GetPosition() - GetFloorPosition();

    // Move the model actor to our position, taking this offset into account.
    // This causes the model's visuals to be exactly at our position.
    mModelActor->SetPosition(GetPosition() + modelVisualOffset);

    // Sync facing helper too.
    if(mModelFacingHelper != nullptr)
    {
        mModelFacingHelper->SetPosition(mModelActor->GetPosition());
    }
}

void GKActor::SetModelRotationToActorRotation()
{
    // Get model's facing direction.
    Vector3 modelDirection = GetModelFacingDirection();

    // Get actor's facing direction.
    Vector3 desiredDirection = GetHeading().ToDirection();
    //Debug::DrawLine(GetPosition(), GetPosition() + modelDirection * 10.0f, Color32::Magenta, 60.0f);
    //Debug::DrawLine(GetPosition(), GetPosition() + desiredDirection * 10.0f, Color32::Magenta, 60.0f);
        
    // Get the angle between the model's direction and the actor's direction.
    float angleRadians = Math::Acos(Math::Clamp(Vector3::Dot(modelDirection, desiredDirection), -1.0f, 1.0f));

    // Use cross product to determine clockwise or counter-clockwise angle between the two.
    if(Vector3::Cross(modelDirection, desiredDirection).y < 0)
    {
        angleRadians *= -1;
    }

    // Rotate model by this angle to now be facing the actor's rotation correctly.
    mModelActor->GetTransform()->RotateAround(GetPosition(), Quaternion(Vector3::UnitY, angleRadians));

    // Sync facing helper too.
    if(mModelFacingHelper != nullptr)
    {
        mModelFacingHelper->SetRotation(mModelActor->GetRotation());
    }
}

void GKActor::RefreshFloorInfo()
{
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr && scene->GetSceneData() != nullptr && scene->GetSceneData()->GetBSP() != nullptr)
    {
        scene->GetSceneData()->GetBSP()->GetFloorInfo(GetPosition(), mFloorHeight, mFloorTexture);
    }
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
