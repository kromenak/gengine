//
// GKProp.cpp
//
// Clark Kromenaker
//
#include "GKProp.h"

#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "VertexAnimator.h"

GKProp::GKProp(bool separateModelActor) : GKObject()
{
    // Create mesh actor with mesh renderer on it.
    if(separateModelActor)
    {
        mModelActor = new Actor();
    }
    else
    {
        mModelActor = this;
    }
    mModelRenderer = mModelActor->AddComponent<MeshRenderer>();
    
    // Create animation player on the same object as the mesh renderer.
    mVertexAnimator = mModelActor->AddComponent<VertexAnimator>();
    
    // GasPlayer will go on the actor itself.
    mGasPlayer = AddComponent<GasPlayer>();
}

std::string GKProp::GetModelName() const
{
    if(mModelRenderer != nullptr)
    {
        Model* model = mModelRenderer->GetModel();
        if(model != nullptr)
        {
            return model->GetNameNoExtension();
        }
    }
    return std::string();
}

void GKProp::StartFidget(GAS* gas)
{
    mGasPlayer->Play(gas);
}

void GKProp::StopFidget(std::function<void()> callback)
{
    mGasPlayer->Stop(callback);
}

void GKProp::StartAnimation(VertexAnimParams& animParams)
{
    // Don't let a GAS anim override a non-GAS anim.
    if(animParams.fromAutoScript && mVertexAnimator->IsPlayingNotAutoscript()) { return; }
    
    // If this is not a GAS anim, pause any running GAS.
    if(!animParams.fromAutoScript)
    {
        mGasPlayer->Pause();
    }
    
    // Set anim stop callback.
    animParams.stopCallback = std::bind(&GKProp::OnVertexAnimationStopInternal, this);
    
    // Start the animation.
    // Note that this will sample the first frame of the animation, updating the model's positions/rotations.
    mVertexAnimator->Start(animParams);
    
    // For absolute anims, position model exactly as specified.
    if(animParams.absolute)
    {
        mModelActor->SetPosition(animParams.absolutePosition);
        mModelActor->SetRotation(animParams.absoluteHeading.ToQuaternion());
    }
    
    // Props don't need any additional logic when anims start, but subclasses (like GKActor) sure do!
    OnVertexAnimationStart(animParams);
}

void GKProp::SampleAnimation(VertexAnimation* anim, int frame)
{
    mVertexAnimator->Sample(anim, frame);
}

void GKProp::StopAnimation(VertexAnimation* anim)
{
    // NOTE: passing nullptr will stop ALL playing animations.
    mVertexAnimator->Stop(anim);
}

void GKProp::OnActive()
{
    // My mesh becomes active when I become active.
    mModelActor->SetActive(true);
}

void GKProp::OnInactive()
{
    // My mesh becomes inactive when I become inactive.
    mModelActor->SetActive(false);
}

void GKProp::OnUpdate(float deltaTime)
{
    /*
    if(mModelRenderer != nullptr)
    {
        mModelRenderer->DebugDrawAABBs();
    }
    */
}

void GKProp::OnVertexAnimationStopInternal()
{
    mGasPlayer->Resume();
    OnVertexAnimationStop();
}