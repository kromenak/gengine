//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"

#include <iostream>

#include "ActionBar.h"
#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "Color32.h"
#include "Debug.h"
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
#include "Walker.h"
#include "WalkerBoundary.h"

extern Mesh* quad;

Scene::Scene(std::string name, std::string timeCode) :
    mGeneralName(name),
	mSpecificName(name + timeCode),
	mSceneData(name, timeCode)
{
	//std::cout << mSpecificName << std::endl;
	
    // Set BSP to be rendered.
    Services::GetRenderer()->SetBSP(mSceneData.GetBSP());
    
    // Figure out if we have a skybox, and set it to be rendered.
    Services::GetRenderer()->SetSkybox(mSceneData.GetSkybox());
    
    // Create game camera.
	mCamera = new GameCamera();
	
	// Create animation player.
	Actor* animationActor = new Actor();
	mAnimationPlayer = animationActor->AddComponent<AnimationPlayer>();
	
	// Position the camera at the the default position and heading.
	SceneCameraData* defaultRoomCamera = mSceneData.GetDefaultRoomCamera();
	if(defaultRoomCamera != nullptr)
	{
    	mCamera->SetPosition(defaultRoomCamera->position);
    	mCamera->SetRotation(Quaternion(Vector3::UnitY, defaultRoomCamera->angle.GetX()));
	}
	
    // Create soundtrack player and get it playing!
	Soundtrack* soundtrack = mSceneData.GetSoundtrack();
	if(soundtrack != nullptr)
	{
		Actor* actor = new Actor();
		SoundtrackPlayer* soundtrackPlayer = actor->AddComponent<SoundtrackPlayer>();
		soundtrackPlayer->Play(soundtrack);
	}
	
	// Create action bar, which will be used to choose nouns/verbs by the player.
	mActionBar = new ActionBar();
	
	// For debugging - render walker bounds overlay on game world.
	{
		WalkerBoundary* walkerBoundary = mSceneData.GetWalkerBoundary();
		if(walkerBoundary != nullptr)
		{
			Actor* walkerBoundaryActor = new Actor();
			
			MeshRenderer* walkerBoundaryMeshRenderer = walkerBoundaryActor->AddComponent<MeshRenderer>();
			walkerBoundaryMeshRenderer->SetMesh(quad);
			
			Material m;
			m.SetDiffuseTexture(walkerBoundary->GetTexture());
			walkerBoundaryMeshRenderer->SetMaterial(0, m);
			
			Vector3 size = walkerBoundary->GetSize();
			Vector3 offset = walkerBoundary->GetOffset();
			offset.SetX(-offset.GetX() + size.GetX() * 0.5f);
			offset.SetZ(-offset.GetY() + size.GetY() * 0.5f);
			offset.SetY(0.1f); // Offset slightly up to avoid z-fighting with floor (in most scenes).
			
			walkerBoundaryActor->SetPosition(offset);
			walkerBoundaryActor->SetRotation(Quaternion(Vector3::UnitX, Math::kPiOver2));
			walkerBoundaryActor->SetScale(size);
		}
	}
}

Scene::~Scene()
{
	Services::GetRenderer()->SetBSP(nullptr);
	Services::GetRenderer()->SetSkybox(nullptr);
}

void Scene::OnSceneEnter()
{
	// Create actors for the scene.
	std::vector<SceneActorData*> sceneActorDatas = mSceneData.GetSceneActorDatas();
	for(auto& actorDef : sceneActorDatas)
	{
		// The actor's 3-letter identifier can be derived from the name of the model.
		std::string identifier;
		if(actorDef->model != nullptr)
		{
			identifier = actorDef->model->GetNameNoExtension();
		}
		
		// Create actor.
		GKActor* actor = new GKActor(identifier);
		mActors.push_back(actor);
		
		// Set noun.
		actor->SetNoun(actorDef->noun);
		
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
		
		// Start in idle state.
		//actor->SetState(GKActor::State::Idle);
		
		// Set up the actor's walker support, if any.
		Model* walkerAidModel = Services::GetAssets()->LoadModel("DOR_" + identifier);
		if(walkerAidModel != nullptr)
		{
			// For walking anims to work correctly, a walker aid actor must exist
			// and be part of the scenes actor list (so an animation can be started on it).
			GKActor* walkerAid = new GKActor();
			mActors.push_back(walkerAid);
			
			// Make the walker aid a child of the actor itself.
			Transform* walkerAidTransform = walkerAid->GetComponent<Transform>();
			walkerAidTransform->SetParent(actor->GetComponent<Transform>());
			walkerAid->GetMeshRenderer()->SetModel(walkerAidModel);
			
			// Give the walker needed references to walk correctly.
			actor->GetWalker()->SetWalkAidMeshRenderer(walkerAid->GetMeshRenderer());
			actor->GetWalker()->SetWalkMeshTransform(actor->GetMeshRenderer()->GetOwner()->GetComponent<Transform>());
		}
		
		//TODO: Apply init anim.
		
		//TODO: If hidden, hide.
		
		// If this is our ego, save a reference to it.
		if(actorDef->ego)
		{
			// If we already created an Ego, delete the old one. Latest one takes priority.
			// This actually happens pretty often - general SIF says Gabe is ego, specific says Grace.
			// Maybe there's a better way to handle this? But this is the easiest for now.
			if(mEgo != nullptr)
			{
				mEgo->Actor::SetState(Actor::State::Dead);
			}
			mEgo = actor;
		}
	}
	
	// Iterate over scene model data and prep the scene.
	// First, we want to hide and scene models that are set to "hidden".
	// Second, we want to spawn any non-scene models.
	std::vector<SceneModelData*> sceneModelDatas = mSceneData.GetSceneModelDatas();
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
					mSceneData.GetBSP()->Hide(modelDef->name);
				}
				break;
			}
				
			// "HitTest" type models should be hidden, but still interactive.
			case SceneModelData::Type::HitTest:
			{
				//std::cout << "Hide " << modelDef->name << std::endl;
				mSceneData.GetBSP()->Hide(modelDef->name);
				break;
			}
				
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
				mActors.push_back(actor);
				
				// If it's a "gas prop", use provided gas as the fidget for the actor.
				if(modelDef->type == SceneModelData::Type::GasProp)
				{
					actor->StartFidget(modelDef->gas);
				}
				break;
			}
				
			default:
				std::cout << "Unaccounted for model type: " << (int)modelDef->type << std::endl;
				break;
		}
	}
	
	// After all models have been created, run through and execute init anims.
	// Want to wait until after creating all actors, in case init anims need to touch created actors!
	for(auto& modelDef : sceneModelDatas)
	{
		// Run any init anims specified.
		// These are usually needed to correctly position the model.
		if((modelDef->type == SceneModelData::Type::Prop || modelDef->type == SceneModelData::Type::GasProp)
		   && modelDef->initAnim != nullptr)
		{
			mAnimationPlayer->Sample(modelDef->initAnim, 0);
		}
	}
	
	// Check for and run "scene enter" actions.
	std::vector<NVC*> nvcs = mSceneData.GetNounVerbCaseSets();
	for(auto& nvc : nvcs)
	{
		const NVCItem* nvcItem = nvc->GetAction("SCENE", "ENTER");
		if(nvcItem != nullptr)
		{
			std::cout << "Executing scene enter for " << nvc->GetName() << std::endl;
			nvcItem->Execute();
		}
	 }
}

void Scene::InitEgoPosition(std::string positionName)
{
    if(mEgo == nullptr) { return; }
    
    ScenePositionData* position = mSceneData.GetScenePosition(positionName);
    if(position == nullptr) { return; }
    
    // Set position and heading.
    mEgo->SetPosition(position->position);
    mEgo->SetRotation(Quaternion(Vector3::UnitY, position->heading));
    
    if(position->camera != nullptr)
    {
        mCamera->SetPosition(position->camera->position);
		
		Quaternion rotation = Quaternion(Vector3::UnitY, position->camera->angle.GetX()) *
		Quaternion(Vector3::UnitX, position->camera->angle.GetY());
        mCamera->SetRotation(rotation);
    }
}

void Scene::SetCameraPosition(std::string cameraName)
{
	SceneCameraData* camera = mSceneData.GetRoomCamera(cameraName);
	if(camera != nullptr)
	{
		mCamera->SetPosition(camera->position);
		
		Quaternion rotation = Quaternion(Vector3::UnitY, camera->angle.GetX()) *
		Quaternion(Vector3::UnitX, camera->angle.GetY());
		mCamera->SetRotation(rotation);
	}
	else
	{
		std::cout << "Could not find camera " << cameraName << std::endl;
	}
}

bool Scene::CheckInteract(const Ray& ray)
{
	BSP* bsp = mSceneData.GetBSP();
	if(bsp == nullptr) { return false; }
	
	HitInfo hitInfo;
	if(!bsp->RaycastNearest(ray, hitInfo)) { return false; }
	
	// If hit the floor, this IS an interaction, but not an interesting one.
	// Clicking will walk the player, but we don't count it as an interactive object.
	if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData.GetFloorModelName()))
	{
		return false;
	}
	
	// See if the hit item matches any scene model data.
	SceneModelData* sceneModelData = nullptr;
	std::vector<SceneModelData*> sceneModelDatas = mSceneData.GetSceneModelDatas();
	for(auto& modelData : sceneModelDatas)
	{
		if(StringUtil::EqualsIgnoreCase(modelData->name, hitInfo.name))
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
	
	BSP* bsp = mSceneData.GetBSP();
	if(bsp == nullptr) { return; }
	
    // Cast ray against scene BSP to see if it intersects with anything.
    // If so, it means we clicked on that thing.
	//TODO: Need to also raycast against models (like Gabe, etc).
	HitInfo hitInfo;
	if(!bsp->RaycastNearest(ray, hitInfo)) { return; }
	//std::cout << "Hit " << hitInfo.name << std::endl;
	
	// Clicked on the floor - move ego to position.
	if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData.GetFloorModelName()))
	{
		// Check walker boundary to see whether we can walk to this spot.
		mEgo->GetWalker()->WalkTo(hitInfo.position, mSceneData.GetWalkerBoundary(), nullptr);
		return;
	}
	
    // Correlate the interacted model name to model data from the SIF.
    // This allows us to correlate a model in the BSP to a noun keyword.
    SceneModelData* sceneModelData = nullptr;
    std::vector<SceneModelData*> sceneModelDatas = mSceneData.GetSceneModelDatas();
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
	
	// If a specific verb is pre-defined for this object, just use that directly.
	if(!sceneModelData->verb.empty())
	{
		std::cout << "Trying to play default verb " << sceneModelData->verb << std::endl;
		const NVCItem* action = mSceneData.GetNounVerbAction(sceneModelData->noun, sceneModelData->verb, mEgo);
		if(action != nullptr)
		{
			action->Execute();
		}
		return;
	}
	
	// Find all verbs that can be used for this object.
	std::vector<const NVCItem*> viableActions = mSceneData.GetViableVerbsForNoun(sceneModelData->noun, mEgo);
	
	// Show the action bar. Internally, this takes care of executing the chosen action.
	mActionBar->Show(viableActions, std::bind(&Scene::ExecuteNVC, this, std::placeholders::_1));
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
	BSP* bsp = mSceneData.GetBSP();
	if(bsp != nullptr)
	{
		HitInfo hitInfo;
		if(bsp->RaycastSingle(downRay, mSceneData.GetFloorModelName(), hitInfo))
		{
			return hitInfo.position.GetY();
		}
	}
	
	// If didn't hit floor, just return 0.
	// TODO: Maybe we should return a default based on the floor BSP's height?
	return 0.0f;
}

GKActor* Scene::GetActorByModelName(std::string modelName)
{
	for(auto& actor : mActors)
	{
		MeshRenderer* meshRenderer = actor->GetMeshRenderer();
		if(meshRenderer != nullptr)
		{
			Model* model = meshRenderer->GetModel();
			if(model != nullptr)
			{
				if(StringUtil::EqualsIgnoreCase(model->GetNameNoExtension(), modelName))
				{
					return actor;
				}
			}
		}
	}
	return nullptr;
}

GKActor* Scene::GetActorByNoun(std::string noun)
{
	for(auto& actor : mActors)
	{
		if(StringUtil::EqualsIgnoreCase(actor->GetNoun(), noun))
		{
			return actor;
		}
	}
	return nullptr;
}

void Scene::ApplyTextureToSceneModel(std::string modelName, Texture* texture)
{
	mSceneData.GetBSP()->SetTexture(modelName, texture);
}

void Scene::ExecuteNVC(const NVCItem* nvc)
{
	// Ignore nulls.
	if(nvc == nullptr) { return; }
	
	// Before executing the NVC, we need to handle any approach.
	std::cout << (int)nvc->approach << std::endl;
	std::cout << nvc->target << std::endl;
	switch(nvc->approach)
	{
		case NVCItem::Approach::WalkTo:
		{
			ScenePositionData* scenePos = mSceneData.GetScenePosition(nvc->target);
			if(scenePos != nullptr)
			{
				mEgo->GetWalker()->WalkTo(scenePos->position, scenePos->heading, mSceneData.GetWalkerBoundary(), [nvc]() -> void {
					nvc->Execute();
				});
			}
			break;
		}
		case NVCItem::Approach::Anim: // Example use: R25 Open/Close Window, R25 Open/Close Dresser
		{
			Animation* anim = Services::GetAssets()->LoadAnimation(nvc->target);
			if(anim != nullptr)
			{
				//TODO: Get position corresponding to first frame of animation and move there.
				nvc->Execute();
			}
			break;
		}
		case NVCItem::Approach::Near: // Never used in GK3.
		{
			std::cout << "Executed NEAR approach type!" << std::endl;
			ScenePositionData* scenePos = mSceneData.GetScenePosition(nvc->target);
			if(scenePos != nullptr)
			{
				mEgo->SetPosition(scenePos->position);
			}
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::NearModel: // Example use: RC1 Bookstore Door, Hallway R25 Door
		{
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::Region: // Only use: RC1 "No Vacancies" Sign
		{
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::TurnTo: // Never used in GK3.
		{
			std::cout << "Executed TURNTO approach type!" << std::endl;
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::TurnToModel: // Example use: R25 Couch Sit, most B25
		{
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::WalkToSee: // Example use: R25 Look Painting/Couch/Dresser, RC1 Look Bench/Bookstore Sign
		{
			nvc->Execute();
			break;
		}
		case NVCItem::Approach::None:
		{
			// Just do it!
			nvc->Execute();
			break;
		}
		default:
		{
			std::cout << "Unaccounted for approach!" << std::endl;
			break;
		}
	}
}
