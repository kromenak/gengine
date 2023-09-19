#include "GKProp.h"

#include "AssetManager.h"
#include "Billboard.h"
#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Quaternion.h"
#include "SceneData.h"
#include "Vector3.h"
#include "VertexAnimator.h"

GKProp::GKProp() : GKObject()
{
    mMeshRenderer = AddComponent<MeshRenderer>();
    mMeshRenderer->SetShader(gAssetManager.LoadShader("3D-Tex-Lit"));

    mVertexAnimator = AddComponent<VertexAnimator>();
    
    mGasPlayer = AddComponent<GasPlayer>();
}

GKProp::GKProp(Model* model) : GKProp()
{
    mMeshRenderer->SetModel(model);

    // Use the model's name as the name of the actor.
    // This is sometimes used for gameplay logic, so be careful about changing this!
    SetName(mMeshRenderer->GetModelName());

    // If this prop acts as a billboard, add the billboard component to it.
    if(model->IsBillboard())
    {
        AddComponent<Billboard>();
    }
}

GKProp::GKProp(const SceneModel& modelDef) : GKProp(modelDef.model)
{
    SetNoun(modelDef.noun);
    SetVerb(modelDef.verb);

    // If it's a "gas prop", use provided gas as the fidget for the actor.
    //TODO: Ideally, start fidget during init, not construction.
    if(modelDef.type == SceneModel::Type::GasProp)
    {
        StartFidget(modelDef.gas);
    }

    // If it should be hidden by default, hide it.
    SetActive(!modelDef.hidden);
}

void GKProp::Init(const SceneData& sceneData)
{
    // We can set these right away becuase props don't really move around the scene (but I guess they could though?).
    for(Material& material : mMeshRenderer->GetMaterials())
    {
        material.SetVector4("uLightPos", Vector4(sceneData.GetGlobalLightPosition(), 1.0f));
        //material.SetColor("uAmbientColor", Color32(75, 75, 75, 0));
        material.SetColor("uAmbientColor", Color32(126, 126, 126, 0));
    }
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
    animParams.stopCallback = std::bind(&GKProp::OnVertexAnimationStop, this);
    
    // Start the animation.
    // Note that this will sample the first frame of the animation, updating the model's positions/rotations.
    mVertexAnimator->Start(animParams);
    
    // For absolute anims, position model exactly as specified.
    if(animParams.absolute)
    {
        SetPosition(animParams.absolutePosition);
        SetRotation(animParams.absoluteHeading.ToQuaternion());
    }
}

void GKProp::SampleAnimation(VertexAnimParams& animParams, int frame)
{
    // Sample the animation on the specified frame.
    mVertexAnimator->Sample(animParams.vertexAnimation, frame);

    // For absolute anims, position model exactly as specified.
    if(animParams.absolute)
    {
        SetPosition(animParams.absolutePosition);
        SetRotation(animParams.absoluteHeading.ToQuaternion());
    }
}

void GKProp::StopAnimation(VertexAnimation* anim)
{
    // NOTE: passing nullptr will stop ALL playing animations.
    mVertexAnimator->Stop(anim);
}

AABB GKProp::GetAABB()
{
    return mMeshRenderer->GetAABB();
}

void GKProp::OnVertexAnimationStop()
{
    mGasPlayer->Resume();
}