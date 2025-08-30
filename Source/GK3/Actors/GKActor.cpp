#include "GKActor.h"

#include "ActionManager.h"
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
#include "PersistState.h"
#include "SceneManager.h"
#include "StringUtil.h"
#include "VertexAnimation.h"
#include "VertexAnimator.h"
#include "Walker.h"

extern Mesh* quad;

namespace
{
    const float kShadowSize = 30.0f;
}

TYPEINFO_INIT(GKActor, GKObject, 33)
{

}

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
    mModelActor = new Actor(GetName() + "_Model");

    // Add 3D model renderer using lit textured shader.
    mMeshRenderer = mModelActor->AddComponent<MeshRenderer>();
    mMeshRenderer->SetShader(gAssetManager.GetShader("LitTexture")); // Shader must be applied first
    mMeshRenderer->SetModel(mActorDef->model);

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

    // Create blob shadow under actor.
    // It must be rotated 90 degrees, since the template quad we're using is on the X/Y plane.
    // It must be scaled to match the approximate scale of the character models in the game.
    mShadowActor = new Actor(GetName() + "_Shadow");
    mShadowActor->SetRotation(Quaternion(Vector3::UnitX, Math::kPiOver2));
    mShadowActor->SetScale(Vector3(kShadowSize, kShadowSize, 0.0f));

    MeshRenderer* shadowMeshRenderer = mShadowActor->AddComponent<MeshRenderer>();
    shadowMeshRenderer->SetMesh(quad);

    // The shadow texture is similar to a BSP lightmap texture, so we'll render it in the same way.
    Material m(gAssetManager.GetShader("LightmapTexture"));
    m.SetDiffuseTexture(&Texture::White);
    m.SetTexture("uLightmap", gAssetManager.LoadSceneTexture("SHADOW.BMP"));
    m.SetVector4("uLightmapScaleOffset", Vector4::One);
    m.SetFloat("uLightmapMultiplier", 2.1f); // higher value makes the shadow less noticeable
    m.SetTranslucent(true);
    shadowMeshRenderer->SetMaterial(0, m);
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
    if(mActorDef->initAnim != nullptr)
    {
        // This is a TURBO HACK right here! In 306P, Mosely/Buchelli/Buthane are sitting in the dining room. But Buthane is missing!?
        // The problem is her initAnim in the SIF is not an absolute anim (it's missing the absolute x/y/z/etc data). So she isn't positioned correctly.
        // There are three possible solutions to this:
        // 1) Somehow, the original game gets this correct. Is my detection of absolute vs relative anims incorrect? Are init anims always absolute? Or did the original game use a similar hack?
        // 2) Catch the initAnim use and replace it with one that works (what I'm doing here).
        // 3) Add a "fixed" version of the asset to the Assets folder.
        Animation* initAnim = mActorDef->initAnim;
        if(StringUtil::EqualsIgnoreCase(GetName(), "MAD") && StringUtil::StartsWithIgnoreCase(initAnim->GetName(), "MADSITGNRIC"))
        {
            initAnim = gAssetManager.LoadAnimation("MadDinFig01", AssetScope::Scene);
        }

        // Sample the init anim.
        gSceneManager.GetScene()->GetAnimator()->Sample(initAnim, 0, mActorDef->model->GetNameNoExtension());
    }
    else
    {
        if(GetName() != "JAM")
        {
            StartFidget(GKActor::FidgetType::Idle);
        }
    }

    // If an init anim was defined, we should make sure to sync the actor to the updated model position/rotation.
    // The init anim can move the model to a very different pose/location, so we want things to be synced before starting another anim.
    SyncActorToModelPositionAndRotation();
}

void GKActor::InitPosition(const ScenePosition* scenePosition)
{
    // Set position and heading from passed in scene position.
    SetPosition(scenePosition->position);
    SetHeading(scenePosition->heading);

    // Make sure we are on the floor.
    SnapToFloor();

    // After some experimenting, another interesting thing "InitEgoPosition" does is stop any current animation and sample the walk anim.
    // This puts the actor in a sane default state.
    mVertexAnimator->Stop(nullptr); // stop all anims

    // Sample walk anim start.
    gSceneManager.GetScene()->GetAnimator()->Sample(mCharConfig->walkStartAnim, 0);
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
        // This seems to help with how some fidgets interact with the walker system.
        SetModelRotationToActorRotation();

        // Play the fidget.
        mGasPlayer->Play(newFidget);
        mActiveFidget = type;
        //printf("%s: starting fidget %s\n", GetNoun().c_str(), newFidget->GetName().c_str());
    }
}

void GKActor::StopFidget(const std::function<void()>& callback)
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

void GKActor::InterruptFidget(bool forTalk, const std::function<void()>& callback)
{
    mGasPlayer->Interrupt(forTalk, callback);
}

void GKActor::TurnTo(const Heading& heading, const std::function<void()>& finishCallback)
{
    InterruptFidget(false, [this, heading, finishCallback](){
        mWalker->WalkToExact(GetPosition(), heading, finishCallback);
    });
}

void GKActor::WalkToBestEffort(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    InterruptFidget(false, [this, position, heading, finishCallback](){
        mWalker->WalkToBestEffort(position, heading, finishCallback);
    });
}

void GKActor::WalkToExact(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    InterruptFidget(false, [this, position, heading, finishCallback](){
        mWalker->WalkToExact(position, heading, finishCallback);
    });
}

void GKActor::WalkToGas(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback)
{
    // This version of the function is needed just to tell the walker if this walk request is coming from a GAS script or not!
    // We also don't need to interrupt fidgets in this case, since the request is coming from the fidget GAS anyways.
    mWalker->WalkToGas(position, heading, finishCallback);
}

void GKActor::WalkToSee(GKObject* target, const std::function<void()>& finishCallback)
{
    InterruptFidget(false, [this, target, finishCallback](){
        mWalker->WalkToSee(target, finishCallback);
    });
}

void GKActor::WalkToAnimationStart(Animation* anim, const std::function<void()>& finishCallback)
{
    // GOAL: walk the actor to the initial position/rotation of this anim.
    // Ideally this should result in a seamless transition from the walk into the animation - it actually works well most of the time!

    // Need a valid anim.
    if(anim == nullptr)
    {
        if(finishCallback != nullptr) { finishCallback(); }
        return;
    }

    // Retrieve vertex animation that matches this actor's model.
    // If no vertex anim exists, we can't walk to animation start! This is probably a developer error.
    VertexAnimNode* vertexAnimNode = anim->GetFirstVertexAnimationForModel(mMeshRenderer->GetModelName());
    if(vertexAnimNode == nullptr)
    {
        if(finishCallback != nullptr) { finishCallback(); }
        return;
    }

    // Sample position/pose on first frame of animation.
    // That gives our walk pos/rotation, BUT it is in the actor's local space.
    Vector3 walkPos = vertexAnimNode->vertexAnimation->SampleVertexPosition(0, mCharConfig->hipAxesMeshIndex, mCharConfig->hipAxesGroupIndex, mCharConfig->hipAxesPointIndex);
    VertexAnimationTransformPose transformPose = vertexAnimNode->vertexAnimation->SampleTransformPose(0, mCharConfig->hipAxesMeshIndex);

    // Also get hip, left shoe, and right shoe positions - all in local space.
    Vector3 hipPos = transformPose.meshToLocalMatrix.GetTranslation();
    Vector3 leftShoePos = vertexAnimNode->vertexAnimation->SampleTransformPose(0, mCharConfig->leftShoeAxesMeshIndex).meshToLocalMatrix.GetTranslation();
    Vector3 rightShoePos = vertexAnimNode->vertexAnimation->SampleTransformPose(0, mCharConfig->rightShoeAxesMeshIndex).meshToLocalMatrix.GetTranslation();

    // If this is an absolute animation, the above position needs to be converted to world space.
    Matrix4 transformMatrix = transformPose.meshToLocalMatrix;
    if(vertexAnimNode->absolute)
    {
        Vector3 animWorldPos = vertexAnimNode->CalcAbsolutePosition();
        Quaternion modelToActorRot(Vector3::UnitY, Math::ToRadians(vertexAnimNode->absoluteWorldToModelHeading));

        Matrix4 localToWorldMatrix = Matrix4::MakeTranslate(animWorldPos) * Matrix4::MakeRotate(modelToActorRot);
        transformMatrix = localToWorldMatrix * transformMatrix;

        hipPos = localToWorldMatrix.TransformPoint(hipPos);
        leftShoePos = localToWorldMatrix.TransformPoint(leftShoePos);
        rightShoePos = localToWorldMatrix.TransformPoint(rightShoePos);
    }

    // Calculate walk pos in world space.
    walkPos = transformMatrix.TransformPoint(walkPos);
    walkPos.y = GetPosition().y;

    // PROBLEM WITH ROTATIONS:
    // Usually actor's face +Z, but their models face -Z. As a result, we'd usually calculate actor heading as "model rotation plus PI".
    // BUT the game is inconsistent about this - in some cases, actors face +Z and the model also faces +Z.
    // So...how can we detect this case? Vector math to the rescue!

    // Imagine a triangle made up of shoe positions and hip position.
    // Calculate the surface normal of the triangle. This is our facing direction.
    Vector3 vec1 = rightShoePos - leftShoePos;
    Vector3 vec2 = hipPos - leftShoePos;
    Vector3 perp = Vector3::Cross(vec1, vec2);
    perp.y = 0.0f;
    Vector3 modelFacingDir = Vector3::Normalize(perp);

    // In case you want to visualize that...
    // The magenta line shows the model facing, the yellow shows the axis direction. Usually, they are close to opposite...but not always.
    //Debug::DrawLine(leftShoePos, rightShoePos, Color32::Blue, 60.0f);
    //Debug::DrawLine(leftShoePos, hipPos, Color32::Red, 60.0f);
    //Debug::DrawLine(hipPos, hipPos + modelFacingDir * 10.0f, Color32::Magenta, 60.0f);
    //Debug::DrawLine(hipPos, hipPos + transformMatrix.GetYAxis() * 10.0f, Color32::Yellow, 60.0);

    // The vast majority of models have their model facing opposite the y-axis: we do "model rotation plus PI" in those cases.
    // In some rare cases, the model facing is the y-axis: we do "just model rotation" in those cases.
    Heading heading = Heading::None;
    if(Vector3::Dot(transformMatrix.GetYAxis(), modelFacingDir) > 0)
    {
        heading = Heading::FromQuaternion(transformMatrix.GetRotation());
    }
    else
    {
        heading = Heading::FromQuaternion(transformMatrix.GetRotation() * Quaternion(Vector3::UnitY, Math::kPi));
    }

    // Walk to that position/heading.
    InterruptFidget(false, [this, walkPos, heading, finishCallback](){
        mWalker->WalkToExact(walkPos, heading, finishCallback);
    });

    // To visualize walk position/heading.
    //Debug::DrawLine(walkPos, walkPos + Vector3::UnitY * 10.0f, Color32::Orange, 10.0f);
    //Debug::DrawLine(walkPos, walkPos + heading.ToDirection() * 10.0f, Color32::Red, 10.0f);
}

Vector3 GKActor::GetWalkDestination() const
{
    if(!mWalker->IsWalking()) { return GetPosition(); }
    return mWalker->GetDestination();
}

void GKActor::SnapToFloor()
{
    if(mFloorHeight != kNoFloorValue)
    {
        Vector3 pos = GetPosition();
        pos.y = mFloorHeight + mCharConfig->shoeThickness;
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
    Vector3 position = GetFloorPosition();
    position.y += mCharConfig->walkerHeight;
    return position;
}

Vector3 GKActor::GetFloorPosition() const
{
    Vector3 leftShoeWorldPos;
    Vector3 rightShoeWorldPos;
    return GetModelFloorAndShoePositions(leftShoeWorldPos, rightShoeWorldPos);
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

    // In the original game, starting a vertex animation on a character will cause any in-progress walk to be cancelled.
    // Of course, the trick is to only do this if the vertex animation being played isn't one of the walk animations!
    // This also seems to not apply to autoscript anims - we don't want to cancel the "turn" part of a walk due to a breathing anim!
    if(!animParams.fromAutoScript && mWalker->IsWalking() && !mWalker->IsWalkAnimation(animParams.vertexAnimation))
    {
        mWalker->StopWalk();
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
}

void GKActor::StopAnimation(VertexAnimation* anim)
{
    // NOTE: passing nullptr will stop ALL playing animations.
    mVertexAnimator->Stop(anim);
}

AABB GKActor::GetAABB() const
{
    return mMeshRenderer->GetAABB();
}

void GKActor::SetShadowEnabled(bool enabled)
{
    mShadowEnabled = enabled;
    mShadowActor->SetActive(enabled);
}

void GKActor::OnPersist(PersistState& ps)
{
    // The scene is loaded/init'd per usual when loading a save game. This can cause GKActors to run their idle fidgets.
    // When loading a game, to avoid fidget callbacks from interfering with load logic, stop and cancel all fidget anims before proceeding.
    if(ps.IsLoading())
    {
        mGasPlayer->StopAndCancelAnims();
    }

    // Shared base class persist logic.
    GKObject::OnPersist(ps);

    // Save/load fidgets being used and which one is active.
    ps.Xfer(PERSIST_VAR(mIdleFidget));
    ps.Xfer(PERSIST_VAR(mTalkFidget));
    ps.Xfer(PERSIST_VAR(mListenFidget));
    ps.Xfer<FidgetType, int>(PERSIST_VAR(mActiveFidget));

    // Save/load shadow enabled state.
    ps.Xfer(PERSIST_VAR(mShadowEnabled));

    // Persist walker state.
    mWalker->OnPersist(ps);

    // If loading, we need to potentially refresh state based on loaded data.
    if(ps.IsLoading())
    {
        mShadowActor->SetActive(mShadowEnabled);

        // Make sure these values are up-to-date, or else they may have stale values from the actor's init code.
        // Remember, we are loading an actor's state after scene load/init code has already executed.
        mStartVertexAnimPosition = GetPosition();
        mStartVertexAnimRotation = GetRotation();

        // Play the active fidget if it's been changed.
        CheckUpdateActiveFidget(mActiveFidget);
    }
}

void GKActor::OnActive()
{
    // My model becomes active when I become active.
    mModelActor->SetActive(true);

    // Same with my shadow.
    mShadowActor->SetActive(mShadowEnabled);
}

void GKActor::OnInactive()
{
    // My model becomes inactive when I become inactive.
    mModelActor->SetActive(false);

    // Same with my shadow.
    mShadowActor->SetActive(false);
}

void GKActor::OnLateUpdate(float deltaTime)
{
    // If the fidget hasn't been set yet, start the idle fidget.
    // TODO: Ideally, it would be great to do this in Init(), but I found it led to some undesirable behavior (see Lady Howard in Museum during Day 1, 10AM for example).
    // TODO: It would be good to dive into that deeper and understand what's happening -
    // TODO: it seems like maybe if an "init anim" is applied, the fidget shouldn't be applied until the VertexAnimator has updated once? Maybe?
    if(mActiveFidget == FidgetType::Unset)
    {
        StartFidget(GKActor::FidgetType::Idle);
    }

    // OPTIONAL: draw axes to visualize Actor position and facing.
    //Debug::DrawAxes(GetTransform()->GetLocalToWorldMatrix());

    // Set actor position/rotation to match model's floor position/rotation.
    SyncActorToModelPositionAndRotation();

    // Have the blob shadow always be underneath the model's floor position.
    // Move the shadow up slightly to avoid z-fighting with floor (or rendering under floor).
    Vector3 leftShoeWorldPos;
    Vector3 rightShoeWorldPos;
    Vector3 floorPosition = GetModelFloorAndShoePositions(leftShoeWorldPos, rightShoeWorldPos);
    floorPosition.y = mFloorHeight + 0.08f;
    mShadowActor->SetPosition(floorPosition);

    // Update the scale of the shadow based on how much floor area the actor is taking up. This is mostly affected by walk animations.
    // We can estimate how big the shadow should be based on shoe positions. Farther apart shoes means more floor covered means bigger shadow.
    AABB aabb = AABB::FromPoints(leftShoeWorldPos, rightShoeWorldPos);

    // The left/right shoe positions are typically pretty small, but they do give the general shape.
    // But add the shadow size to this to give shadow buffer around the edges.
    Vector3 aabbSizeWithBuffer = aabb.GetSize() + Vector3(kShadowSize, 0.0f, kShadowSize);
    mShadowActor->SetScale(Vector3(aabbSizeWithBuffer.x, aabbSizeWithBuffer.z, 1.0f));

    // Can be used to visualize the size of the final shadow rectangle on the floor.
    /*
    aabbSizeWithBuffer.y = 0.0f;
    Debug::DrawAABB(AABB::FromCenterAndSize(aabb.GetCenter(), aabbSizeWithBuffer), Color32::Red);
    */

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

    // When a vertex animation stops/ends, make sure the actor position/rotation match the final position/rotation of the model.
    // This is most important when action-skipping. It ensures you end up in the same position/rotation whether you skipped or didn't skip.
    // This sync action usually occurs in LateUpdate, but due to action skip using higher delta time, we may get many vertex anims starting/stopping without LateUpdate being called.
    if(gActionManager.IsSkippingCurrentAction())
    {
        SyncActorToModelPositionAndRotation();
    }
}

Vector3 GKActor::GetModelFacingDirection() const
{
    if(mForcedFacingDir != Vector3::Zero)
    {
        return mForcedFacingDir;
    }

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
            //Debug::DrawLine(GetPosition(), mModelFacingHelper->GetPosition(), Color32::White);
        }
        */

        // SCENARIO A: We have a model facing helper, and it is currently in use (it's animating or we are walking).
        if(mModelFacingHelper->GetComponent<VertexAnimator>()->IsPlaying() || mWalker->IsWalkingExceptTurn())
        {
            // Use those three points to calculate a facing direction. The "point" of the arrow is pt 1.
            // OF COURSE these points aren't consistent...Mosely uses different ones.
            if(StringUtil::EqualsIgnoreCase(GetName(), "MOS") || StringUtil::EqualsIgnoreCase(GetName(), "DEM"))
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
        // Unfortunately, not all actors are created equal - if there's a global way to determine facing direction, I haven't found it yet.
        // As needed for individual actors, add logic here!
        if(StringUtil::EqualsIgnoreCase(GetName(), "CAT"))
        {
            Vector3 meshHipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
            Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);

            Vector3 pt2 = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(0);
            Vector3 pt2World = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(pt2);
            facingDir = worldHipPos - pt2World;
        }
        else
        {
            // Get the hip axis, convert y-axis to a facing direction.
            // Remember, models are facing down negative z-axis, so need to negate the axis we get back here.
            Matrix4 hipMeshToWorldMatrix = mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix();
            if(StringUtil::EqualsIgnoreCase(GetName(), "EM2"))
            {
                facingDir = hipMeshToWorldMatrix.GetYAxis();
            }
            else
            {
                facingDir = -hipMeshToWorldMatrix.GetYAxis();
            }
        }
    }

    // Get rid of any height in the facing direction.
    facingDir.y = 0.0f;
    facingDir.Normalize();

    //Debug::DrawLine(GetPosition(), GetPosition() + facingDir * 10.0f, Color32::Blue);
    return facingDir;
}

Vector3 GKActor::GetModelFloorAndShoePositions(Vector3& leftShoeWorldPos, Vector3& rightShoeWorldPos) const
{
    // Get hip pos and convert to world space.
    // To do this, multiply the mesh->local with local->world to get a mesh->world matrix for transforming.
    Vector3 meshHipPos = mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetSubmesh(mCharConfig->hipAxesGroupIndex)->GetVertexPosition(mCharConfig->hipAxesPointIndex);
    Vector3 worldHipPos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->hipAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshHipPos);

    // Get left/right shoe positions.
    Vector3 meshLeftShoePos = mMeshRenderer->GetMesh(mCharConfig->leftShoeAxesMeshIndex)->GetSubmesh(mCharConfig->leftShoeAxesGroupIndex)->GetVertexPosition(mCharConfig->leftShoeAxesPointIndex);
    Vector3 worldLeftShoePos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->leftShoeAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshLeftShoePos);
    leftShoeWorldPos = worldLeftShoePos;

    Vector3 meshRightShoePos = mMeshRenderer->GetMesh(mCharConfig->rightShoeAxesMeshIndex)->GetSubmesh(mCharConfig->rightShoeAxesGroupIndex)->GetVertexPosition(mCharConfig->rightShoeAxesPointIndex);
    Vector3 worldRightShoePos = (mModelActor->GetTransform()->GetLocalToWorldMatrix() * mMeshRenderer->GetMesh(mCharConfig->rightShoeAxesMeshIndex)->GetMeshToLocalMatrix()).TransformPoint(meshRightShoePos);
    rightShoeWorldPos = worldRightShoePos;

    // Use left/right shoe positions to determine our floor position.
    // We're assuming that the actor's lowest foot, minus show thickness, is the floor height.
    Vector3 floorPos = worldHipPos;
    floorPos.y = Math::Min(worldLeftShoePos.y, worldRightShoePos.y);
    floorPos.y -= mCharConfig->shoeThickness;
    return floorPos;
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
    //Debug::DrawLine(GetPosition(), GetPosition() + modelDirection * 10.0f, Color32::Red, 10.0f);
    //Debug::DrawLine(GetPosition(), GetPosition() + desiredDirection * 10.0f, Color32::Green, 10.0f);

    // Get the angle between the model's direction and the actor's direction.
    float angleRadians = Math::Acos(Vector3::Dot(modelDirection, desiredDirection));

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

void GKActor::SyncActorToModelPositionAndRotation()
{
    // Set actor position to match model's floor position.
    Vector3 leftShoeWorldPos;
    Vector3 rightShoeWorldPos;
    Vector3 floorPosition = GetModelFloorAndShoePositions(leftShoeWorldPos, rightShoeWorldPos);
    GetTransform()->SetPosition(floorPosition);

    // Set actor facing direction to match the model's facing direction.
    {
        // Get mine and model's facing directions.
        Vector3 myFacingDir = GetHeading().ToDirection();
        Vector3 modelFacingDir = GetModelFacingDirection();

        // Calculate angle I should rotate to match model's direction.
        // Use cross product to determine clockwise or counter-clockwise rotation.
        float angleRadians = Math::Acos(Vector3::Dot(myFacingDir, modelFacingDir));
        if(Vector3::Cross(myFacingDir, modelFacingDir).y < 0)
        {
            angleRadians *= -1;
        }

        // Rotate desired direction and amount.
        GetTransform()->Rotate(Vector3::UnitY, angleRadians);
    }
}

void GKActor::RefreshFloorInfo()
{
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        bool success = scene->GetFloorInfo(GetPosition(), mFloorHeight, mFloorTexture);
        if(!success)
        {
            mFloorHeight = kNoFloorValue;
        }
    }
}

GAS* GKActor::GetGasForFidget(FidgetType type)
{
    GAS* gas = nullptr;
    switch(type)
    {
    default:
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
        // If the gas file for the active fidget has changed to null, that forces fidget to stop.
        // Otherwise, we just immediately play the new gas file.
        GAS* fidget = GetGasForFidget(changedType);
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
