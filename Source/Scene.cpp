//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"

#include <iostream>

#include "GameCamera.h"
#include "GKActor.h"
#include "Math.h"
#include "MeshRenderer.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "UIImage.h"

extern Mesh* quad;

Scene::Scene(std::string name, std::string timeCode) :
    mGeneralName(name)
{
    // Generate name for specific SIF.
    mSpecificName = name + timeCode;
    
    // Load general and specific SIF assets.
    mGeneralSIF = Services::GetAssets()->LoadSIF(mGeneralName);
    mSpecificSIF = Services::GetAssets()->LoadSIF(mSpecificName);
    
    // Load scene data asset.
    mSceneData = Services::GetAssets()->LoadSceneData(mGeneralSIF->GetSceneDataName());
    
    // Load BSP and set it to be rendered.
    mSceneBSP = Services::GetAssets()->LoadBSP(mSceneData->GetBSPName());
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
    std::vector<SceneActorData*> sceneActorDatas = mGeneralSIF->GetSceneActorDatas();
    for(auto& actorDef : sceneActorDatas)
    {
        // Create actor.
        GKActor* actor = new GKActor();
        
        //TODO: Associate noun with actor.
        
        // Set actor's initial position and rotation.
        if(actorDef->position != nullptr)
        {
            Vector3 position = actorDef->position->position;
            actor->SetPosition(position);
            actor->SetRotation(Quaternion(Vector3::UnitY, actorDef->position->heading));
        }
        
        // Set actor's graphical appearance.
        actor->GetMeshRenderer()->SetModel(actorDef->model);
        
        // Save actor's GAS references.
        actor->SetIdleGas(actorDef->idleGas);
        actor->SetTalkGas(actorDef->talkGas);
        actor->SetListenGas(actorDef->listenGas);
        
        // Always start in "idle" state.
        actor->SetState(GKActor::State::Idle);
        
        //TODO: Apply init anim.
        
        //TODO: If hidden, hide.
        
        // If this is our ego, save a reference to it.
        if(actorDef->ego)
        {
            mEgo = actor;
        }
    }
    
    // Iterate over scene model data and prep the scene.
    // First, we want to hide and scene models that are set to "hidden".
    // Second, we want to spawn any non-scene models.
    std::vector<SceneModelData*> sceneModelDatas = mGeneralSIF->GetSceneModelDatas();
    for(auto& modelDef : sceneModelDatas)
    {
        if(modelDef->type == SceneModelData::Type::Scene)
        {
            if(modelDef->hidden)
            {
                mSceneBSP->Hide(modelDef->name);
            }
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
	
	//Texture* uiTex = Services::GetAssets()->LoadTexture("B25WALKERBOUNDS.BMP");
    //Actor* uiActor = new Actor();
	//uiActor->AddComponent<UIImage>()->SetTexture(uiTex);
	
	Actor* walkerBoundaryActor = new Actor();
	
	MeshRenderer* walkerBoundaryMeshRenderer = walkerBoundaryActor->AddComponent<MeshRenderer>();
	walkerBoundaryMeshRenderer->SetMesh(quad);
	
	Material m;
	m.SetDiffuseTexture(mGeneralSIF->GetWalkBoundaryTexture());
	walkerBoundaryMeshRenderer->SetMaterial(0, m);
	
	
	Vector3 size = mGeneralSIF->GetWalkBoundarySize();
	Vector3 offset = mGeneralSIF->GetWalkBoundaryOffset();
	offset.SetX(-offset.GetX() + size.GetX() * 0.5f);
	offset.SetZ(-offset.GetY() + size.GetY() * 0.5f);
	offset.SetY(0.0f);
	
	walkerBoundaryActor->SetPosition(offset);
	walkerBoundaryActor->SetRotation(Quaternion(Vector3::UnitX, Math::kPiOver2));
	walkerBoundaryActor->SetScale(size);
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
	HitInfo hitInfo;
	if(!mSceneBSP->RaycastNearest(ray, hitInfo)) { return; }
	
	// Clicked on the floor - move ego to position.
	if(hitInfo.name == mGeneralSIF->GetFloorBspModelName())
	{
		Texture* tex = mGeneralSIF->GetWalkBoundaryTexture();
		Vector3 size = mGeneralSIF->GetWalkBoundarySize();
		Vector3 offset = mGeneralSIF->GetWalkBoundaryOffset();
		
		Vector3 pos = hitInfo.position;
		std::cout << "World Pos: " << pos << std::endl;
		
		pos.SetX(pos.GetX() + offset.GetX());
		pos.SetZ(pos.GetZ() + offset.GetY());
		std::cout << "Offset Pos: " << pos << std::endl;
		
		pos.SetX(pos.GetX() / size.GetX());
		pos.SetZ(pos.GetZ() / size.GetY());
		std::cout << "Normalized Pos: " << pos << std::endl;
		
		pos.SetX(pos.GetX() * tex->GetWidth());
		pos.SetZ(pos.GetZ() * tex->GetHeight());
		std::cout << "Pixel Pos: " << pos << std::endl;
		
		mEgo->SetPosition(hitInfo.position);
		return;
	}
	
    // Correlate the interacted model name to model data from the SIF.
    // This allows us to correlate a model in the BSP to a noun keyword.
    SceneModelData* sceneModelData = nullptr;
    std::vector<SceneModelData*> sceneModelDatas = mGeneralSIF->GetSceneModelDatas();
    for(auto& modelData : sceneModelDatas)
    {
        if(modelData->name == hitInfo.name)
        {
            sceneModelData = modelData;
            break;
        }
    }
    
    // If we couldn't find any scene model data for this model, we're done.
    if(sceneModelData == nullptr) { return; }
    
    //TODO: Here is where we'd stop and show the UI for the user to pick a verb!
    // Let's assume a verb for the moment (LOOK).
    
    // Next, does any NVC defines an entry for the given noun/verb combo?
    std::vector<NVC*> nvcs = mGeneralSIF->GetNounVerbCases();
    for(auto& nvc : nvcs)
    {
        NVCItem* item = nvc->GetNVC(sceneModelData->noun, "LOOK");
        if(item != nullptr)
        {
			// If so, check if the case for the item is met (Gabe Only, Grace Only, Only After 5PM, etc).
			if(nvc->IsCaseMet(item))
			{
				//TODO: Deal with approach and target.
				
				// Execute the sheep script for this NVC.
				item->Execute();
				
				// Only execute one per interaction.
				// If multiple NVC items match, only the first will be played.
				break;
			}
        }
    }
}

float Scene::GetFloorY(const Vector3& position)
{
	// Calculate ray origin using passed position, but really high in the air!
	Vector3 rayOrigin = position;
	rayOrigin.SetY(10000);
	
	// Create ray with origin high in the sky and pointing straight down.
	Ray downRay(rayOrigin, -Vector3::UnitY);
	
	// Raycast straight down and test against the floor BSP.
	// If we hit something, just use the Y hit position as the floor's Y.
	HitInfo hitInfo;
	if(mSceneBSP->RaycastSingle(downRay, mGeneralSIF->GetFloorBspModelName(), hitInfo))
	{
		return hitInfo.position.GetY();
	}
	
	// If didn't hit floor, just return 0.
	// TODO: Maybe we should return a default based on the floor BSP's height?
	return 0.0f;
}
