//
// GKProp.h
//
// Clark Kromenaker
//
// A "prop" is a dynamic 3D object in the scene that can animate and be interacted with.
//
#pragma once
#include "GKObject.h"

class GAS;
class GasPlayer;
class MeshRenderer;
class VertexAnimator;
struct VertexAnimParams;

class GKProp : public GKObject
{
public:
    GKProp(bool isActor = false);
    
    void SetHeading(const Heading& heading) override;
    
    std::string GetModelName() const;
    
    void StartFidget(GAS* gas);
    void StopFidget();
    
    void StartAnimation(VertexAnimParams& animParams);
    void SampleAnimation(VertexAnimation* anim, int frame);
    void StopAnimation(VertexAnimation* anim = nullptr);
    
    MeshRenderer* GetMeshRenderer() const { return mModelRenderer; }
    VertexAnimator* GetVertexAnimator() const { return mVertexAnimator; }
    GasPlayer* GetGasPlayer() const { return mGasPlayer; }
	
protected:
    void OnActive() override;
    void OnInactive() override;
	void OnUpdate(float deltaTime) override;
    
    // Model is attached to a separate actor so it can move separately.
    // GK3 anims often have model origin != object origin, so moving separately is needed.
    Actor* mModelActor = nullptr;
    MeshRenderer* mModelRenderer = nullptr;
    
    // Many objects animate using vertex animations.
    VertexAnimator* mVertexAnimator = nullptr;
    
    // GAS player allows object to animate in an automated/scripted fashion based on some simple command statements.
    GasPlayer* mGasPlayer = nullptr;
    
    // A rotation applied to the mesh internally to get it facing the right way.
    // For example, GKActor meshes have forward facing -Z axis, so need to be rotated PI radians.
    Quaternion mMeshLocalRotation = Quaternion::Identity;
    
    virtual void OnVertexAnimationStart(const VertexAnimParams& animParams) { }
    virtual void OnVertexAnimationStop() { }
    
private:
    void OnVertexAnimationStopInternal() { OnVertexAnimationStop(); }
};
