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

Stage::Stage(std::string name, int day, int hour) :
    mGeneralName(name)
{
    //TODO: Maybe there should be a utility function to do this, or maybe enum is better or something?
    std::string ampm = (hour <= 11) ? "A" : "P";
    if(hour > 12)
    {
        hour -= 12;
    }
    
    mSpecificName = name + std::to_string(day) + std::to_string(hour) + ampm;
    std::cout << mGeneralName << ", " << mSpecificName << std::endl;
    
    mGeneralSIF = Services::GetAssets()->LoadSIF(mGeneralName + ".SIF");
    mScene = Services::GetAssets()->LoadScene(mGeneralSIF->GetSCNName() + ".SCN");
    
    // Load BSP and set it to be rendered.
    mSceneBSP = Services::GetAssets()->LoadBSP(mScene->GetBSPName() + ".BSP");
    Services::GetRenderer()->SetBSP(mSceneBSP);
    
    std::vector<ActorDefinition*> actorDefinitions = mGeneralSIF->GetActorDefinitions();
    for(auto& actorDef : actorDefinitions)
    {
        Actor* actor = new Actor();
        if(actorDef->position != nullptr)
        {
            Vector3 position = actorDef->position->position;
            actor->SetPosition(position);
        }
        
        MeshComponent* meshComponent = new MeshComponent(actor);
        meshComponent->SetModel(actorDef->model);
        actor->AddComponent(meshComponent);
    }
}
