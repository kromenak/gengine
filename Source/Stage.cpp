//
// Stage.cpp
//
// Clark Kromenaker
//
#include "Stage.h"
#include <iostream>
#include "Services.h"
#include "Actor.h"
#include "MeshComponent.h"
#include "SoundtrackPlayer.h"
#include "GameCamera.h"
#include "Math.h"

Stage::Stage(std::string name, int day, int hour) :
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
    
    // Load scene asset.
    mScene = Services::GetAssets()->LoadScene(mGeneralSIF->GetSCNName() + ".SCN");
    
    // Load BSP and set it to be rendered.
    mSceneBSP = Services::GetAssets()->LoadBSP(mScene->GetBSPName() + ".BSP");
    Services::GetRenderer()->SetBSP(mSceneBSP);
    
    // Position room camera.
    SceneCamera* defaultRoomCamera = mGeneralSIF->GetDefaultRoomCamera();
    GameCamera* gameCamera = new GameCamera();
    gameCamera->SetPosition(defaultRoomCamera->position);
    gameCamera->SetRotation(Quaternion(Vector3::UnitY, defaultRoomCamera->angle.GetX()));
    
    // Create actors for the scene.
    std::vector<ActorDefinition*> actorDefinitions = mGeneralSIF->GetActorDefinitions();
    for(auto& actorDef : actorDefinitions)
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
