#include "GKProp.h"

#include "AssetManager.h"
#include "Billboard.h"
#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "PersistState.h"
#include "SceneData.h"
#include "VertexAnimator.h"

TYPEINFO_INIT(GKProp, GKObject, 32)
{

}

GKProp::GKProp() : GKObject()
{
    mMeshRenderer = AddComponent<MeshRenderer>();
    mMeshRenderer->SetShader(gAssetManager.GetShader("LitTexture"));

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
    if(model != nullptr && model->IsBillboard())
    {
        AddComponent<Billboard>();
    }
}

GKProp::GKProp(const SceneModel& modelDef) : GKProp(modelDef.model)
{
    SetNoun(modelDef.noun);
    SetVerb(modelDef.verb);

    // If it's a "gas prop", use provided gas as the fidget for the actor.
    if(modelDef.type == SceneModel::Type::GasProp)
    {
        mFidgetGas = modelDef.gas;
    }

    // If it should be hidden by default, hide it.
    SetActive(!modelDef.hidden);

    // If a fixed lighting color was specified, set it right away.
    if(modelDef.fixedLightingColor != Color32::Magenta)
    {
        SetFixedLightingColor(modelDef.fixedLightingColor);
    }
}

void GKProp::Init(const SceneData& sceneData)
{
    // We can set these right away becuase props don't really move around the scene (but I guess they could though?).
    for(Material& material : mMeshRenderer->GetMaterials())
    {
        material.SetVector4("uLightPos", Vector4(sceneData.GetGlobalLightPosition(), 1.0f));

        // Only set ambient light color if not already set (due to fixed lighting color applied in constructor).
        if(material.GetColor("uAmbientColor") == nullptr)
        {
            material.SetColor("uAmbientColor", Color32(126, 126, 126));
        }
    }

    // Play this prop's GAS, if any.
    if(mFidgetGas != nullptr)
    {
        StartFidget(mFidgetGas);
    }
}

void GKProp::StartFidget(GAS* gas)
{
    mGasPlayer->Play(gas);
}

void GKProp::StartFidget()
{
    mGasPlayer->Play(mFidgetGas);
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

AABB GKProp::GetAABB() const
{
    return mMeshRenderer->GetAABB();
}

void GKProp::OnVertexAnimationStop()
{
    mGasPlayer->Resume();
}