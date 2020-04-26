//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"

#include <iostream>
#include <limits>

#include "ActionManager.h"
#include "Animator.h"
#include "CharacterManager.h"
#include "Color32.h"
#include "Debug.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "GMath.h"
#include "MeshRenderer.h"
#include "Mover.h"
#include "RectTransform.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"
#include "Walker.h"
#include "WalkerBoundary.h"

extern Mesh* quad;

Scene::Scene(const std::string& name, const std::string& timeblock) : Scene(name, Timeblock(timeblock))
{
    
}

Scene::Scene(const std::string& name, const Timeblock& timeblock) :
	mLocation(name),
	mTimeblock(timeblock)
{
	// Create game camera.
	mCamera = new GameCamera();
	
	// Create animation player.
	Actor* animationActor = new Actor();
	mAnimator = animationActor->AddComponent<Animator>();
}

Scene::~Scene()
{
	if(mSceneData != nullptr)
	{
		Unload();
	}
}

void Scene::Load()
{
	// If this is true, we are calling load when scene is already loaded!
	if(mSceneData != nullptr)
	{
		//TODO: Ignore for now, but maybe we should Unload and then re-Load?
		return;
	}
	
	// Creating scene data loads SIFs, but does nothing else yet!
	mSceneData = new SceneData(mLocation, mTimeblock.ToString());
	
	// It's generally important that we know how our "ego" will be as soon as possible.
	// This is because the scene loading *itself* may check who ego is to do certain things!
	const SceneActor* egoSceneActor = mSceneData->DetermineWhoEgoWillBe();
	//TODO: If no ego, I guess we fail loading!?
	if(egoSceneActor == nullptr)
	{
		std::cout << "No ego actor could be predicted for scene!" << std::endl;
	}
	else
	{
		mEgoName = egoSceneActor->noun;
	}
	
	// Set location.
	Services::Get<LocationManager>()->SetLocation(mLocation);
	
	// Increment location counter IMMEDIATELY.
	// We know this b/c various scripts that need to run on "1st time enter" or similar check if count==1.
	// For those to evaluate correctly, we need to do this BEFORE we even parse scene data or anything.
	Services::Get<LocationManager>()->IncLocationCount(mEgoName, mLocation, mTimeblock);
	
	// Based on location, timeblock, and game progress, resolve what data we will load into the current scene.
	// After calling this, SceneData will have interpreted all data from SIFs and determined exactly what we should and should not load/use for the scene right now.
	mSceneData->ResolveSceneData();
	
	// Set BSP to be rendered.
    Services::GetRenderer()->SetBSP(mSceneData->GetBSP());
    
    // Figure out if we have a skybox, and set it to be rendered.
    Services::GetRenderer()->SetSkybox(mSceneData->GetSkybox());
	
	// Position the camera at the the default position and heading.
	const SceneCamera* defaultRoomCamera = mSceneData->GetDefaultRoomCamera();
	if(defaultRoomCamera != nullptr)
	{
    	mCamera->SetPosition(defaultRoomCamera->position);
    	mCamera->SetRotation(Quaternion(Vector3::UnitY, defaultRoomCamera->angle.GetX()));
	}
	
	// If a camera bounds model exists for this scene, pass it along to the camera.
	Model* cameraBoundsModel = Services::GetAssets()->LoadModel(mSceneData->GetCameraBoundsModelName());
	if(cameraBoundsModel != nullptr)
	{
		mCamera->SetBounds(cameraBoundsModel);
		
		// For debugging - we can visualize the camera bounds mesh, if desired.
		//Actor* cameraBoundsActor = new Actor();
		//MeshRenderer* cameraBoundsMeshRenderer = cameraBoundsActor->AddComponent<MeshRenderer>();
		//cameraBoundsMeshRenderer->SetModel(cameraBoundsModel);
	}
	
	// Create soundtrack player and get it playing!
	Soundtrack* soundtrack = mSceneData->GetSoundtrack();
	if(soundtrack != nullptr)
	{
		Actor* actor = new Actor();
		mSoundtrackPlayer = actor->AddComponent<SoundtrackPlayer>();
		mSoundtrackPlayer->Play(soundtrack);
	}
	
	// For debugging - render walker bounds overlay on game world.
	//TODO: Move to construction system!
	{
		WalkerBoundary* walkerBoundary = mSceneData->GetWalkerBoundary();
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
	
	// Create actors for the scene.
	const std::vector<const SceneActor*>& sceneActorDatas = mSceneData->GetActors();
	for(auto& actorDef : sceneActorDatas)
	{
		// NEVER spawn an ego who is not our current ego!
		if(actorDef->ego && actorDef != egoSceneActor) { continue; }
		
		// The actor's 3-letter identifier (GAB, GRA, etc) can be derived from the name of the model.
		std::string identifier;
		if(actorDef->model != nullptr)
		{
			identifier = actorDef->model->GetNameNoExtension();
		}
		
		// Create actor.
		GKActor* actor = new GKActor(identifier);
		mActors.push_back(actor);
		mObjects.push_back(actor);
		
		// Set noun (GABRIEL, GRACE, etc).
		actor->SetNoun(actorDef->noun);
		
		// Set actor's initial position and rotation.
		if(!actorDef->positionName.empty())
		{
			const ScenePosition* scenePos = mSceneData->GetScenePosition(actorDef->positionName);
			if(scenePos != nullptr)
			{
				actor->SetPosition(scenePos->position);
				actor->SetHeading(scenePos->heading);
			}
			else
			{
				std::cout << "Invalid position for actor: " << actorDef->positionName << std::endl;
			}
		}
		
		// Set actor's graphical appearance.
		actor->GetMeshRenderer()->SetModel(actorDef->model);
		
		// Save actor's GAS references.
		actor->SetIdleGas(actorDef->idleGas);
		actor->SetTalkGas(actorDef->talkGas);
		actor->SetListenGas(actorDef->listenGas);
		
		// Start in idle state.
		actor->StartFidget(GKActor::FidgetType::Idle);
		
		//TODO: Apply init anim.
		
		//TODO: If hidden, hide.
		
		// If this is our ego, save a reference to it.
		if(actorDef->ego && actorDef == egoSceneActor)
		{
			mEgo = actor;
		}
	}
	
	// Iterate over scene model data and prep the scene.
	// First, we want to hide and scene models that are set to "hidden".
	// Second, we want to spawn any non-scene models.
	const std::vector<const SceneModel*> sceneModelDatas = mSceneData->GetModels();
	for(auto& modelDef : sceneModelDatas)
	{
		switch(modelDef->type)
		{
			// "Scene" type models are ones that are baked into the BSP geometry.
			case SceneModel::Type::Scene:
			{
				// If it should be hidden by default, tell the BSP to hide it.
				if(modelDef->hidden)
				{
					mSceneData->GetBSP()->SetVisible(modelDef->name, false);
				}
				break;
			}
				
			// "HitTest" type models should be hidden, but still interactive.
			case SceneModel::Type::HitTest:
			{
				//std::cout << "Hide " << modelDef->name << std::endl;
				mSceneData->GetBSP()->SetVisible(modelDef->name, false);
				break;
			}
				
			// "Prop" and "GasProp" models both render their own model geometry.
			// Only difference for a "GasProp" is that it uses a provided Gas file too.
			case SceneModel::Type::Prop:
			case SceneModel::Type::GasProp:
			{
				// Create actor.
				GKActor* prop = new GKActor();
				prop->SetNoun(modelDef->noun);
				
				// Set model.
				prop->GetMeshRenderer()->SetModel(modelDef->model);
				mProps.push_back(prop);
				mObjects.push_back(prop);
				
				// If it's a "gas prop", use provided gas as the fidget for the actor.
				if(modelDef->type == SceneModel::Type::GasProp)
				{
					prop->SetIdleGas(modelDef->gas);
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
		if((modelDef->type == SceneModel::Type::Prop || modelDef->type == SceneModel::Type::GasProp)
		   && modelDef->initAnim != nullptr)
		{
			mAnimator->Sample(modelDef->initAnim, 0);
		}
	}
	
	// Check for and run "scene enter" actions.
	const std::vector<NVC*>& nvcs = Services::Get<ActionManager>()->GetActionSets();
	for(auto& nvc : nvcs)
	{
		const Action* action = nvc->GetAction("SCENE", "ENTER");
		if(action != nullptr)
		{
			std::cout << "Executing scene enter for " << nvc->GetName() << std::endl;
			action->Execute();
		}
	}
}

void Scene::Unload()
{
	Services::GetRenderer()->SetBSP(nullptr);
	Services::GetRenderer()->SetSkybox(nullptr);
	
	delete mSceneData;
	mSceneData = nullptr;
}

void Scene::InitEgoPosition(const std::string& positionName)
{
    if(mEgo == nullptr) { return; }
    
	// Get position.
    const ScenePosition* position = GetPosition(positionName);
    if(position == nullptr) { return; }
    
    // Set position and heading.
    mEgo->SetPosition(position->position);
    mEgo->SetHeading(position->heading);
	
	// Should also set camera position/angle.
	// Output a warning if specified position has no camera though.
	if(position->cameraName.empty())
	{
		Services::GetReports()->Log("Warning", "No camera information is supplied in position '" + positionName + "'.");
		return;
	}
	
	// Move the camera to desired position/angle.
	SetCameraPosition(position->cameraName);
}

void Scene::SetCameraPosition(const std::string& cameraName)
{
	// Find camera or fail.
	// Any *named* camera type is valid.
	const SceneCamera* camera = mSceneData->GetRoomCamera(cameraName);
	if(camera == nullptr)
	{
		camera = mSceneData->GetCinematicCamera(cameraName);
	}
	
	// If couldn't find a camera with this name, error out!
	if(camera == nullptr)
	{
		Services::GetReports()->Log("Error", "Error: '" + cameraName + "' is not a valid room camera.");
		return;
	}
	
	// Set position/angle.
	mCamera->SetPosition(camera->position);
	mCamera->SetAngle(camera->angle);
}

bool Scene::CheckInteract(const Ray& ray) const
{
	// Check against any dynamic actors before falling back on BSP check.
	for(auto& object : mObjects)
	{
		MeshRenderer* meshRenderer = object->GetMeshRenderer();
		if(meshRenderer != nullptr && meshRenderer->Raycast(ray))
		{
			return true;
		}
	}
	
	BSP* bsp = mSceneData->GetBSP();
	if(bsp == nullptr) { return false; }
	
	HitInfo hitInfo;
	if(!bsp->RaycastNearest(ray, hitInfo)) { return false; }
	
	// If hit the floor, this IS an interaction, but not an interesting one.
	// Clicking will walk the player, but we don't count it as an interactive object.
	if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData->GetFloorModelName()))
	{
		return false;
	}
	
	// See if the hit item matches any scene model data.
	const SceneModel* sceneModelData = nullptr;
	const std::vector<const SceneModel*>& sceneModelDatas = mSceneData->GetModels();
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
	if(Services::Get<ActionManager>()->IsActionBarShowing()) { return; }
	
	// Also ignore scene interaction when inventory is up.
	if(Services::Get<InventoryManager>()->IsInventoryShowing()) { return; }
	
	// Check against any dynamic actors before falling back on BSP check.
	float nearestActorDistSq = FLT_MAX;
	GKActor* interactedActor = nullptr;
	for(auto& actor : mObjects)
	{
		MeshRenderer* meshRenderer = actor->GetMeshRenderer();
		if(meshRenderer != nullptr && meshRenderer->Raycast(ray))
		{
			// Interacted actor is one closest to me.
			// Might not be 100% accurate, but probably good enough.
			// TODO: (Would also be great if Raycast call returned distance stat for this).
			float actorDistSq = (mCamera->GetPosition() - actor->GetPosition()).GetLengthSq();
			if(actorDistSq < nearestActorDistSq)
			{
				nearestActorDistSq = actorDistSq;
				interactedActor = actor;
			}
		}
	}
	
	// Show the action bar. Internally, this takes care of executing the chosen action.
	if(interactedActor != nullptr)
	{
		Services::Get<ActionManager>()->ShowActionBar(interactedActor->GetNoun(), std::bind(&Scene::ExecuteAction, this, std::placeholders::_1));
		return;
	}
	
	// FROM HERE: we are interacting with static scene objects (BSP).
	// Make sure we have valid BSP.
	BSP* bsp = mSceneData->GetBSP();
	if(bsp == nullptr) { return; }
	
    // Cast ray against scene BSP to see if it intersects with anything.
    // If so, it means we clicked on that thing.
	HitInfo hitInfo;
	if(!bsp->RaycastNearest(ray, hitInfo)) { return; }
	//std::cout << "Hit " << hitInfo.name << std::endl;
	
	// Clicked on the floor - move ego to position.
	if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData->GetFloorModelName()))
	{
		// Check walker boundary to see whether we can walk to this spot.
		mEgo->GetWalker()->WalkTo(hitInfo.position, mSceneData->GetWalkerBoundary(), nullptr);
		return;
	}
	
    // Correlate the interacted model name to model data from the SIF.
    // This allows us to correlate a model in the BSP to a noun keyword.
    const SceneModel* sceneModelData = nullptr;
    const std::vector<const SceneModel*>& sceneModelDatas = mSceneData->GetModels();
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
		const Action* action = Services::Get<ActionManager>()->GetAction(sceneModelData->noun, sceneModelData->verb, mEgo);
		if(action != nullptr)
		{
			action->Execute();
		}
		return;
	}
	
	// Show the action bar for this noun.
	Services::Get<ActionManager>()->ShowActionBar(sceneModelData->noun, std::bind(&Scene::ExecuteAction, this, std::placeholders::_1));
}

float Scene::GetFloorY(const Vector3& position) const
{
	// Calculate ray origin using passed position, but really high in the air!
	Vector3 rayOrigin = position;
	rayOrigin.SetY(10000);
	
	// Create ray with origin high in the sky and pointing straight down.
	Ray downRay(rayOrigin, -Vector3::UnitY);
	
	// Raycast straight down and test against the floor BSP.
	// If we hit something, just use the Y hit position as the floor's Y.
	BSP* bsp = mSceneData->GetBSP();
	if(bsp != nullptr)
	{
		HitInfo hitInfo;
		if(bsp->RaycastSingle(downRay, mSceneData->GetFloorModelName(), hitInfo))
		{
			return hitInfo.position.GetY();
		}
	}
	
	// If didn't hit floor, just return 0.
	// TODO: Maybe we should return a default based on the floor BSP's height?
	return 0.0f;
}

GKActor* Scene::GetSceneObjectByModelName(const std::string& modelName) const
{
	for(auto& object : mObjects)
	{
		MeshRenderer* meshRenderer = object->GetMeshRenderer();
		if(meshRenderer != nullptr)
		{
			Model* model = meshRenderer->GetModel();
			if(model != nullptr)
			{
				if(StringUtil::EqualsIgnoreCase(model->GetNameNoExtension(), modelName))
				{
					return object;
				}
			}
		}
	}
	return nullptr;
}

GKActor* Scene::GetActorByNoun(const std::string& noun) const
{
	for(auto& actor : mActors)
	{
		if(StringUtil::EqualsIgnoreCase(actor->GetNoun(), noun))
		{
			return actor;
		}
	}
	Services::GetReports()->Log("Error", "Error: Who the hell is '" + noun + "'?");
	return nullptr;
}

const ScenePosition* Scene::GetPosition(const std::string& positionName) const
{
	const ScenePosition* position = nullptr;
	if(mSceneData != nullptr)
	{
		position = mSceneData->GetScenePosition(positionName);
	}
	if(position == nullptr)
	{
		Services::GetReports()->Log("Error", "Error: '" + positionName + "' is not a valid position. Call DumpPositions() to see valid positions.");
	}
	return position;
}

void Scene::ApplyTextureToSceneModel(const std::string& modelName, Texture* texture)
{
	mSceneData->GetBSP()->SetTexture(modelName, texture);
}

void Scene::SetSceneModelVisibility(const std::string& modelName, bool visible)
{
	mSceneData->GetBSP()->SetVisible(modelName, visible);
}

bool Scene::IsSceneModelVisible(const std::string& modelName) const
{
	return mSceneData->GetBSP()->IsVisible(modelName);
}

bool Scene::DoesSceneModelExist(const std::string& modelName) const
{
	return mSceneData->GetBSP()->Exists(modelName);
}

void Scene::ExecuteAction(const Action* action)
{
	// Ignore nulls.
	if(action == nullptr) { return; }
	
	// Log to "Actions" stream.
	Services::GetReports()->Log("Actions", "Playing NVC " + action->ToString());
	
	// Before executing the NVC, we need to handle any approach.
	switch(action->approach)
	{
		case Action::Approach::WalkTo:
		{
			const ScenePosition* scenePos = mSceneData->GetScenePosition(action->target);
			if(scenePos != nullptr)
			{
				mEgo->GetWalker()->WalkTo(scenePos->position, scenePos->heading, mSceneData->GetWalkerBoundary(), [action]() -> void {
					action->Execute();
				});
			}
			break;
		}
		case Action::Approach::Anim: // Example use: R25 Open/Close Window, R25 Open/Close Dresser
		{
			Animation* anim = Services::GetAssets()->LoadAnimation(action->target);
			if(anim != nullptr)
			{
				std::cout << "WalkToAnim" << std::endl;
				mEgo->WalkToAnimationStart(anim, mSceneData->GetWalkerBoundary(), [action]() -> void {
					action->Execute();
				});
			}
			break;
		}
		case Action::Approach::Near: // Never used in GK3.
		{
			std::cout << "Executed NEAR approach type!" << std::endl;
			const ScenePosition* scenePos = mSceneData->GetScenePosition(action->target);
			if(scenePos != nullptr)
			{
				mEgo->SetPosition(scenePos->position);
			}
			action->Execute();
			break;
		}
		case Action::Approach::NearModel: // Example use: RC1 Bookstore Door, Hallway R25 Door
		{
			action->Execute();
			break;
		}
		case Action::Approach::Region: // Only use: RC1 "No Vacancies" Sign
		{
			action->Execute();
			break;
		}
		case Action::Approach::TurnTo: // Never used in GK3.
		{
			std::cout << "Executed TURNTO approach type!" << std::endl;
			action->Execute();
			break;
		}
		case Action::Approach::TurnToModel: // Example use: R25 Couch Sit, most B25
		{
			action->Execute();
			break;
		}
		case Action::Approach::WalkToSee: // Example use: R25 Look Painting/Couch/Dresser, RC1 Look Bench/Bookstore Sign
		{
			action->Execute();
			break;
		}
		case Action::Approach::None:
		{
			// Just do it!
			action->Execute();
			break;
		}
		default:
		{
			Services::GetReports()->Log("Error", "Invalid approach " + std::to_string(static_cast<int>(action->approach)));
			action->Execute();
			break;
		}
	}
}
