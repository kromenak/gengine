//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"
#include <iostream>
#include "Services.h"
#include "Actor.h"
#include "MeshComponent.h"
#include "SoundtrackPlayer.h"
#include "GameCamera.h"
#include "Math.h"

Scene::Scene(std::string name, int day, int hour) :
    mGeneralName(name)
{
    //TODO: Maybe there should be a utility function to do this, or maybe enum is better or something?
    std::string ampm = (hour <= 11) ? "A" : "P";
    if(hour > 12) { hour -= 12; }
    
    // Generate name for specific SIF.
    mSpecificName = name + std::to_string(day) + std::to_string(hour) + ampm;
    
    // Load general and specific SIF assets.
    mGeneralSIF = Services::GetAssets()->LoadSIF(mGeneralName + ".SIF");
    mSpecificSIF = Services::GetAssets()->LoadSIF(mSpecificName + ".SIF");
    
    // Load scene data asset.
    mSceneData = Services::GetAssets()->LoadScene(mGeneralSIF->GetSCNName() + ".SCN");
    
    // Load BSP and set it to be rendered.
    mSceneBSP = Services::GetAssets()->LoadBSP(mSceneData->GetBSPName() + ".BSP");
    Services::GetRenderer()->SetBSP(mSceneBSP);
    
    // Figure out if we have a skybox, and set it to be rendered.
    mSkybox = mSceneData->GetSkybox();
    if(mSkybox == nullptr)
    {
        mSkybox = mSpecificSIF->GetSkybox();
    }
    if(mSkybox == nullptr)
    {
        mSkybox = mGeneralSIF->GetSkybox();
    }
    Services::GetRenderer()->SetSkybox(mSkybox);
    
    // Create camera and position it at the the default position and heading.
    SceneCameraData* defaultRoomCamera = mGeneralSIF->GetDefaultRoomCamera();
    mCamera = new GameCamera();
    mCamera->SetPosition(defaultRoomCamera->position);
    mCamera->SetRotation(Quaternion(Vector3::UnitY, defaultRoomCamera->angle.GetX()));
    
    // Create actors for the scene.
    std::vector<SceneActorData*> SceneActorDatas = mGeneralSIF->GetSceneActorDatas();
    for(auto& actorDef : SceneActorDatas)
    {
        Actor* actor = new Actor();
        if(actorDef->position != nullptr)
        {
            Vector3 position = actorDef->position->position;
            actor->SetPosition(position);
            actor->SetRotation(Quaternion(Vector3::UnitY, actorDef->position->heading));
        }
        
        MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();
        meshComponent->SetModel(actorDef->model);
        
        // If this is our ego, save a reference to it.
        if(actorDef->ego)
        {
            mEgo = actor;
        }
    }
    
    // Create soundtrack player and get it playing!
    std::vector<Soundtrack*> soundtracks = mGeneralSIF->GetSoundtracks();
    if(soundtracks.size() == 0 && mSpecificSIF != nullptr)
    {
        soundtracks = mSpecificSIF->GetSoundtracks();
    }
    if(soundtracks.size() > 0)
    {
        Actor* actor = new Actor();
        SoundtrackPlayer* soundtrackPlayer = actor->AddComponent<SoundtrackPlayer>();
        soundtrackPlayer->Play(soundtracks[0]);
    }
}

void Scene::InitEgoPosition(std::string positionName)
{
    if(mEgo == nullptr) { return; }
    
    ScenePositionData* position = mGeneralSIF->GetPosition(positionName);
    if(position == nullptr) { return; }
    
    // Set position and heading.
    mEgo->SetPosition(position->position);
    mEgo->SetRotation(Quaternion(Vector3::UnitY, position->heading));
    
    if(position->camera != nullptr)
    {
        mCamera->SetPosition(position->camera->position);
        mCamera->SetRotation(Quaternion(Vector3::UnitY, position->camera->angle.GetX()));
    }
    else
    {
        //TODO: Output a warning.
    }
}

void Scene::Interact(const Ray& ray)
{
    // Cast ray against scene BSP to see if it intersects with anything.
    // If so, it means we clicked on that thing.
    std::string* name = mSceneBSP->Intersects(ray);
    if(name == nullptr) { return; }
    
    // Correlate the interacted model name to model data from the SIF.
    // This allows us to correlate a model in the BSP to a noun keyword.
    SceneModelData* sceneModelData = nullptr;
    std::vector<SceneModelData*> sceneModelDatas = mGeneralSIF->GetSceneModelDatas();
    for(auto& modelData : sceneModelDatas)
    {
        if(modelData->name == *name)
        {
            sceneModelData = modelData;
            break;
        }
    }
    
    // If we couldn't find any scene model data for this model, we're done.
    if(sceneModelData == nullptr) { return; }
    
    //TODO: Here is where we'd stop and show the UI for the user to pick a verb!
    // Let's assume a verb for the moment (LOOK).
    
    // The next question is whether any NVC defines an entry for the given noun/verb combo.
    std::vector<NVC*> nvcs = mGeneralSIF->GetNounVerbCases();
    for(auto& nvc : nvcs)
    {
        NVCItem* item = nvc->GetNVC(sceneModelData->noun, "LOOK");
        if(item != nullptr)
        {
            if(item->script != nullptr)
            {
                SheepVM vm;
                vm.Execute(item->script);
            }
        }
    }
}
