//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"

#include <iostream>

#include "ActionBar.h"
#include "CharacterManager.h"
#include "Color32.h"
#include "GameCamera.h"
#include "GKActor.h"
#include "Math.h"
#include "MeshRenderer.h"
#include "Mover.h"
#include "RectTransform.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

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
		actor->SetNoun(actorDef->noun);
		
		// The actor's 3-letter identifier can be derived from the name of the model.
		std::string identifier;
		if(actorDef->model != nullptr)
		{
			identifier = actorDef->model->GetNameNoExtension();
		}
		actor->SetIdentifier(identifier);
        
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
		
		//CharacterConfig& characterConfig = Services::Get<CharacterManager>()->GetCharacterConfig(actorDef->model->GetNameNoExtension());
		//actor->PlayAnimation(characterConfig.walkStartTurnLeftAnim);
		
        //TODO: Apply init anim.
        
        //TODO: If hidden, hide.
        
        // If this is our ego, save a reference to it.
        if(actorDef->ego)
        {
            mEgo = actor;
        }
		
		// Save in created actors list.
		mActors.push_back(actor);
    }
	
    // Iterate over scene model data and prep the scene.
    // First, we want to hide and scene models that are set to "hidden".
    // Second, we want to spawn any non-scene models.
    std::vector<SceneModelData*> sceneModelDatas = mGeneralSIF->GetSceneModelDatas();
    for(auto& modelDef : sceneModelDatas)
    {
		switch(modelDef->type)
		{
			// "Scene" type models are ones that are baked into the BSP geometry.
			case SceneModelData::Type::Scene:
			{
				// If it should be hidden by default, tell the BSP to hide it.
				if(modelDef->hidden)
				{
					mSceneBSP->Hide(modelDef->name);
				}
				break;
			}
				
			// "HitTest" type models should be hidden, but still interactive.
			//case SceneModelData::Type::HitTest:
			//	mSceneBSP->Hide(modelDef->name);
			//	break;
				
			// "Prop" and "GasProp" models both render their own model geometry.
			// Only difference for a "GasProp" is that it uses a provided Gas file too.
			case SceneModelData::Type::Prop:
			case SceneModelData::Type::GasProp:
			{
				// Create actor.
				GKActor* actor = new GKActor();
				actor->SetNoun(modelDef->noun);
				
				// Set model.
				actor->GetMeshRenderer()->SetModel(modelDef->model);
				
				// Run any init anims specified.
				// These are usually needed to correctly position the model.
				/*
				if(modelDef->initAnim != nullptr)
				{
					actor->PlayInitAnimation(modelDef->initAnim);
				}
				*/
				
				// For some reason, prop positions are exactly mirrored across the Z-axis from actual desired position.
				// This might be related to negation of z-components when importing the model.
				//actor->SetScale(Vector3(1.0f, 1.0f, -1.0f));
				break;
			}
				
			default:
				std::cout << "Unaccounted for model type: " << (int)modelDef->type << std::endl;
				break;
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
	
	// Create action bar, which will be used to choose nouns/verbs by the player.
	mActionBar = new ActionBar();
	
	// For debugging - render walker bounds overlay on game world.
	/*
	{
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
		offset.SetY(0.1f); // Offset slightly up to avoid z-fighting with floor (in most scenes).
		
		walkerBoundaryActor->SetPosition(offset);
		walkerBoundaryActor->SetRotation(Quaternion(Vector3::UnitX, Math::kPiOver2));
		walkerBoundaryActor->SetScale(size);
	}
	*/
	
	// Check for and run "scene enter" actions.
}

Scene::~Scene()
{
	Services::GetRenderer()->SetBSP(nullptr);
	Services::GetRenderer()->SetSkybox(nullptr);
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

bool Scene::CheckInteract(const Ray& ray)
{
	HitInfo hitInfo;
	if(!mSceneBSP->RaycastNearest(ray, hitInfo)) { return false; }
	
	// If hit the floor, this IS an interaction, but not an interesting one.
	// Clicking will walk the player, but we don't count it as an interactive object.
	if(StringUtil::EqualsIgnoreCase(hitInfo.name, mGeneralSIF->GetFloorBspModelName()))
	{
		return false;
	}
	
	// See if the hit item matches any scene model data.
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
	
	// If we found something, it counts as an interactive thing.
	return sceneModelData != nullptr;
}

void Scene::Interact(const Ray& ray)
{
	// Ignore scene interaction while the action bar is showing.
	if(mActionBar->IsShowing()) { return; }
	
    // Cast ray against scene BSP to see if it intersects with anything.
    // If so, it means we clicked on that thing.
	//TODO: Need to also raycast against models (like Gabe, etc).
	HitInfo hitInfo;
	if(!mSceneBSP->RaycastNearest(ray, hitInfo)) { return; }
	std::cout << "Hit " << hitInfo.name << std::endl;
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
		
		// The color of the pixel at pos seems to indicate whether that spot is walkable.
		// White = totally OK to walk 				(255, 255, 255)
		// Blue = OK to walk						(0, 0, 255)
		// Green = sort of OK to walk 				(0, 255, 0)
		// Red = getting less OK to walk 			(255, 0, 0)
		// Yellow = sort of not OK to walk 			(255, 255, 0)
		// Magenta = really pushing it here 		(255, 0, 255)
		// Grey = pretty not OK to walk here 		(128, 128, 128)
		// Cyan = this is your last warning, buddy 	(0, 255, 255)
		// Black = totally not OK to walk 			(0, 0, 0)
		
		// Need to flip the Y because the calculated value is from lower-left. But X/Y are from upper-left.
		Color32 color = tex->GetPixelColor32(pos.GetX(), tex->GetHeight() - pos.GetZ());
		std::cout << color << std::endl;
		
		// Don't allow walking in black area.
		if(color == Color32::Black)
		{
			std::cout << "Can't walk!" << std::endl;
			return;
		}
		
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
	
	//TODO: If a specific verb is pre-defined for this object, just use that directly.
	
	// Find all verbs that can be used for this object.
	std::vector<const NVCItem*> viableActions;
	std::vector<NVC*> nvcs = mGeneralSIF->GetNounVerbCases();
	for(auto& nvc : nvcs)
	{
		const std::vector<NVCItem>& allActions = nvc->GetActionsForNoun(sceneModelData->noun);
		for(auto& action : allActions)
		{
			if(nvc->IsCaseMet(&action))
			{
				viableActions.push_back(&action);
			}
		}
	}
	
	// Show the action bar. Internally, this takes care of executing the chosen action.
	mActionBar->Show(viableActions);
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
