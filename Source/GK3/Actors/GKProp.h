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
    
    // The actor/mesh renderer used to render this object's model.
    // For props, model actor == this. But for characters, model actor is a separate actor.
    Actor* mModelActor = nullptr;
    MeshRenderer* mModelRenderer = nullptr;
    
    // A rotation applied to the mesh internally to get it facing the right way.
    // For example, GKActor meshes have forward facing -Z axis, so need to be rotated PI radians.
    Quaternion mMeshLocalRotation = Quaternion::Identity;
    
    // Many objects animate using vertex animations.
    VertexAnimator* mVertexAnimator = nullptr;
    
    // GAS player allows object to animate in an automated/scripted fashion based on some simple command statements.
    GasPlayer* mGasPlayer = nullptr;
    
    virtual void OnVertexAnimationStart(const VertexAnimParams& animParams) { }
    virtual void OnVertexAnimationStop() { }
    
private:
    void OnVertexAnimationStopInternal() { OnVertexAnimationStop(); }
};
