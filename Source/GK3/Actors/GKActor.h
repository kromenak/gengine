//
// Clark Kromenaker
//
// A GK "actor" is a humanoid character or animal.
// Actors can walk around, animate, perform facial expressions and eye contact, etc.
//
#pragma once
#include "GKObject.h"

#include <cfloat>
#include <string>

#include "Heading.h"
#include "Walker.h"

struct CharacterConfig;
class FaceController;
class GAS;
class GasPlayer;
class Model;
class PersistState;
struct SceneActor;
class SceneData;
struct ScenePosition;
class VertexAnimation;
class VertexAnimator;
struct VertexAnimParams;

class GKActor : public GKObject
{
    TYPEINFO_SUB(GKActor, GKObject);
public:
    GKActor(const SceneActor* actorDef);

    void Init(const SceneData& sceneData);
    void InitPosition(const ScenePosition* scenePosition);

    // Fidgets
    enum class FidgetType
    {
        Unset = -1, // The fidget has not yet ever been set.
        None,
        Idle,
        Talk,
        Listen
    };
    void StartFidget(FidgetType type);
    void StopFidget(const std::function<void()>& callback = nullptr);
    FidgetType GetCurrentFidgetType() const { return mActiveFidget; }

    void SetIdleFidget(GAS* fidget);
    void SetTalkFidget(GAS* fidget);
    void SetListenFidget(GAS* fidget);
    GAS* GetIdleFidget() { return mIdleFidget; }
    GAS* GetTalkFidget() { return mTalkFidget; }
    GAS* GetListenFidget() { return mListenFidget; }

    void InterruptFidget(bool forTalk, const std::function<void()>& callback);

    // Walker
    void TurnTo(const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToBestEffort(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToExact(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToGas(const Vector3& position, const Heading& heading, const std::function<void()>& finishCallback);
    void WalkToSee(GKObject* target, const std::function<void()>& finishCallback);
    void WalkToAnimationStart(Animation* anim, const std::function<void()>& finishCallback);
    Vector3 GetWalkDestination() const;
    Walker* GetWalker() const { return mWalker; }

    // Floors
    const std::string& GetShoeType() const;
    void SnapToFloor();
    Vector3 GetFloorPosition() const;
    Texture* GetFloorTypeWalkingOn() const { return mFloorTexture; }

    // Face/Head
    FaceController* GetFaceController() const { return mFaceController; }
    Vector3 GetHeadPosition() const;

    void SetPosition(const Vector3& position);
    void Rotate(float rotationAngle);
    void SetHeading(const Heading& heading) override;

    void StartAnimation(VertexAnimParams& animParams) override;
    void SampleAnimation(VertexAnimParams& animParams, int frame) override;
    void StopAnimation(VertexAnimation* anim = nullptr) override;
    MeshRenderer* GetMeshRenderer() const override { return mMeshRenderer; }
    AABB GetAABB() const override;

    const CharacterConfig* GetConfig() const { return mCharConfig; }

    void SetModelFacingHelper(GKProp* helper) { mModelFacingHelper = helper; }
    void SetForcedFacingDir(const Vector3& dir) { mForcedFacingDir = dir; }

    void SetShadowEnabled(bool enabled);

    void OnPersist(PersistState& ps) override;

protected:
    void OnActive() override;
    void OnInactive() override;
    void OnLateUpdate(float deltaTime) override;

private:
    // The "definition" used to construct this GKActor.
    const SceneActor* mActorDef = nullptr;

    // The character's configuration, which defines helpful parameters for controlling the actor.
    const CharacterConfig* mCharConfig = nullptr;

    // The actor's walking control.
    Walker* mWalker = nullptr;

    // Currently detected floor height and texture.
    const float kNoFloorValue = FLT_MAX;
    float mFloorHeight = kNoFloorValue;
    Texture* mFloorTexture = nullptr;

    // Is this actor's shadow enabled?
    bool mShadowEnabled = true;

    // The actor's face control.
    FaceController* mFaceController = nullptr;

    // The fidget the actor is currently playing.
    FidgetType mActiveFidget = FidgetType::Unset;

    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleFidget = nullptr;
    GAS* mTalkFidget = nullptr;
    GAS* mListenFidget = nullptr;

    // GAS player is used to execute fidgets.
    GasPlayer* mGasPlayer = nullptr;

    // The actor/mesh renderer used to render this object's model.
    Actor* mModelActor = nullptr;
    MeshRenderer* mMeshRenderer = nullptr;

    // Often, we can calculate the facing of the model from the model itself.
    // Sometimes however, GK3 uses a helper object that animates alongside an animation to track the facing direction.
    GKProp* mModelFacingHelper = nullptr;
    Vector3 mForcedFacingDir = Vector3::Zero;

    // Many objects animate using vertex animations.
    VertexAnimator* mVertexAnimator = nullptr;

    // Vertex anims often change the position of the mesh, but that doesn't mean the actor's position should change.
    // Sometimes we allow a vertex anim to affect the actor's position.
    bool mVertexAnimAllowMove = false;

    // For non-move vertex anims, the actor resets to its original position/rotation when the anim stops.
    // So, we must save the start position/rotation for that purpose.
    Vector3 mStartVertexAnimPosition;
    Quaternion mStartVertexAnimRotation;

    // A blob shadow that displays under the actor.
    Actor* mShadowActor = nullptr;

    void OnVertexAnimationStop();

    Vector3 GetModelFacingDirection() const;
    Vector3 GetModelFloorAndShoePositions(Vector3& leftShoeWorldPos, Vector3& rightShoeWorldPos) const;
    void SetModelPositionToActorPosition();
    void SetModelRotationToActorRotation();

    void SyncActorToModelPositionAndRotation();

    void RefreshFloorInfo();

    GAS* GetGasForFidget(FidgetType type);
    void CheckUpdateActiveFidget(FidgetType changedType);
};
