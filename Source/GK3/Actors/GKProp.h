//
// Clark Kromenaker
//
// A "prop" is a dynamic 3D object in the scene that can animate and be interacted with.
//
#pragma once
#include "GKObject.h"

#include <functional>

class GAS;
class GasPlayer;
class MeshRenderer;
class Model;
class PersistState;
class SceneData;
struct SceneModel;
class VertexAnimator;
struct VertexAnimParams;

class GKProp : public GKObject
{
    TYPEINFO_SUB(GKProp, GKObject);
public:
    GKProp();
    GKProp(Model* model);
    GKProp(const SceneModel& modelDef);

    void Init(const SceneData& sceneData);

    void StartFidget(GAS* gas);
    void StartFidget();
    void StopFidget(std::function<void()> callback = nullptr);

    void StartAnimation(VertexAnimParams& animParams) override;
    void SampleAnimation(VertexAnimParams& animParams, int frame) override;
    void StopAnimation(VertexAnimation* anim = nullptr) override;
    MeshRenderer* GetMeshRenderer() const override { return mMeshRenderer; }
    AABB GetAABB() const override;

private:
    // The prop's mesh renderer.
    MeshRenderer* mMeshRenderer = nullptr;

    // Props can animate.
    VertexAnimator* mVertexAnimator = nullptr;

    // Autoscripts can drive animation on "GAS Props".
    GasPlayer* mGasPlayer = nullptr;

    // If this is a "GAS Prop," the gas being used/played.
    GAS* mFidgetGas = nullptr;

    void OnVertexAnimationStop();
};
