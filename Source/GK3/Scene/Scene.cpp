#include "Scene.h"

#include <iostream>
#include <limits>

#include "ActionBar.h"
#include "ActionManager.h"
#include "Animator.h"
#include "BSPActor.h"
#include "CharacterManager.h"
#include "Collisions.h"
#include "Color32.h"
#include "Debug.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "GKProp.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "GMath.h"
#include "MeshRenderer.h"
#include "Profiler.h"
#include "RectTransform.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StatusOverlay.h"
#include "StringUtil.h"
#include "Walker.h"
#include "WalkerBoundary.h"

std::string Scene::mEgoName;

/*static*/ const char* Scene::GetEgoName()
{
    return mEgoName.c_str();
}

Scene::Scene(const std::string& name, const std::string& timeblock) :
    Scene(name, Timeblock(timeblock))
{
    
}

Scene::Scene(const std::string& name, const Timeblock& timeblock) :
	mLocation(name),
	mTimeblock(timeblock),
    mLayer(this)
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
    TIMER_SCOPED("Scene::Load");

	// If this is true, we are calling load when scene is already loaded!
	if(mSceneData != nullptr)
	{
		//TODO: Ignore for now, but maybe we should Unload and then re-Load?
		return;
	}

    // Scene layer is now active!
    Services::Get<LayerManager>()->PushLayer(&mLayer);
    
	// Creating scene data loads SIFs, but does nothing else yet!
	mSceneData = new SceneData(mLocation, mTimeblock.ToString());
	
	// It's generally important that we know how our "ego" will be as soon as possible.
	// This is because the scene loading *itself* may check who ego is to do certain things!
	const SceneActor* egoSceneActor = mSceneData->DetermineWhoEgoWillBe();
	if(egoSceneActor != nullptr)
	{
        mEgoName = egoSceneActor->noun;
	}

    // Set location.
    Services::Get<LocationManager>()->SetLocation(mLocation);

    // Create status overlay actor.
    // Do this after setting location so it shows the correct location!
    mStatusOverlay = new StatusOverlay();
    
    // Based on location, timeblock, and game progress, resolve what data we will load into the current scene.
    // Do this BEFORE incrementing location count, as SIF conditions sometimes do "zero-checks" (e.g. if current location count == 0).
    // After this, SceneData will have combined all SIFs and parsed all conditions to determine exactly what actors/models/etc to used right now.
    mSceneData->ResolveSceneData();

    // Increment location counter after resolving scene data.
    // Despite SIFs sometimes doing "zero-checks," other NVC and SHP scripts typically do "one-checks".
    // E.g. run on "1st time enter" checks count == 1.
    Services::Get<LocationManager>()->IncLocationCount(mEgoName, mLocation, mTimeblock);
	
	// Set BSP to be rendered.
    Services::GetRenderer()->SetBSP(mSceneData->GetBSP());
    
    // Figure out if we have a skybox, and set it to be rendered.
    Services::GetRenderer()->SetSkybox(mSceneData->GetSkybox());
	
	// Position the camera at the the default position and heading.
	const SceneCamera* defaultRoomCamera = mSceneData->GetDefaultRoomCamera();
	if(defaultRoomCamera != nullptr)
	{
    	mCamera->SetPosition(defaultRoomCamera->position);
    	mCamera->SetRotation(Quaternion(Vector3::UnitY, defaultRoomCamera->angle.x));
	}

    // Force a camera update to make sure the audio listener is positioned correctly in the scene.
    // This stops audio playing too loudly on scene load if the audio listener hasn't yet updated.
    mCamera->Update(0.0f);
	
	// If a camera bounds model exists for this scene, pass it along to the camera.
    for(auto& modelName : mSceneData->GetCameraBoundsModelNames())
    {
        Model* model = Services::GetAssets()->LoadModel(modelName);
        if(model != nullptr)
        {
            mCamera->AddBounds(model);
        }
    }
    
	// Create actors for the scene.
	const std::vector<const SceneActor*>& sceneActorDatas = mSceneData->GetActors();
	for(auto& actorDef : sceneActorDatas)
	{
		// NEVER spawn an ego who is not our current ego!
		if(actorDef->ego && actorDef != egoSceneActor) { continue; }
		
		// Create actor.
		GKActor* actor = new GKActor(*actorDef);
		mActors.push_back(actor);
		mPropsAndActors.push_back(actor);
		
		// If this is ego, save a reference to it.
		if(actorDef->ego && actorDef == egoSceneActor)
		{
			mEgo = actor;
		}

        /*
        //TODO: GK3 has these "DOR" models (Direction of Rotation?) that mimic movement during walk animations.
        //TODO: Really unclear whether they are actually needed or not.
        Model* walkerDorModel = Services::GetAssets()->LoadModel("DOR_" + actor->GetMeshRenderer()->GetModelName());
        if(walkerDorModel != nullptr)
        {
            GKProp* walkerDOR = new GKProp();
            walkerDOR->GetMeshRenderer()->SetModel(walkerDorModel);
            mPropsAndActors.push_back(walkerDOR);
        }
        */
	}
	
	// Iterate over scene model data and prep the scene.
	// First, we want to hide and scene models that are set to "hidden".
	// Second, we want to spawn any non-scene models.
	const std::vector<const SceneModel*>& sceneModelDatas = mSceneData->GetModels();
	for(auto& modelDef : sceneModelDatas)
	{
		switch(modelDef->type)
		{
			// "Scene" type models are ones that are baked into the BSP geometry.
			case SceneModel::Type::Scene:
			{
				BSPActor* actor = mSceneData->GetBSP()->CreateBSPActor(modelDef->name);
                if(actor == nullptr) { break; }
				mBSPActors.push_back(actor);
				
				actor->SetNoun(modelDef->noun);
				actor->SetVerb(modelDef->verb);
				
				// If it should be hidden by default, tell the BSP to hide it.
				if(modelDef->hidden)
				{
					actor->SetActive(false);
				}
				break;
			}
				
			// "HitTest" type models should be hidden, but still interactive.
			case SceneModel::Type::HitTest:
			{
				BSPActor* actor = mSceneData->GetBSP()->CreateBSPActor(modelDef->name);
                if(actor == nullptr) { break; }
				mBSPActors.push_back(actor);
                mHitTestActors.push_back(actor);
				
				actor->SetNoun(modelDef->noun);
				actor->SetVerb(modelDef->verb);
				
				// Hit tests, if hidden, are completely deactivated.
				// However, if not hidden, they are still not visible, but they DO receive ray casts.
				if(modelDef->hidden)
				{
					actor->SetActive(false);
				}
				else
				{
					actor->SetVisible(false);
				}
				break;
			}
				
			// "Prop" and "GasProp" models both render their own model geometry.
			// Only difference for a "GasProp" is that it uses a provided Gas file too.
			case SceneModel::Type::Prop:
			case SceneModel::Type::GasProp:
			{
				GKProp* prop = new GKProp(*modelDef);
				mProps.push_back(prop);
				mPropsAndActors.push_back(prop);
				break;
			}

			default:
				std::cout << "Unaccounted for model type: " << (int)modelDef->type << std::endl;
				break;
		}
	}

    // Init construction system.
    mConstruction.Init(this, mSceneData);
}

void Scene::Unload()
{
	Services::GetRenderer()->SetBSP(nullptr);
	Services::GetRenderer()->SetSkybox(nullptr);
	
	delete mSceneData;
	mSceneData = nullptr;

    while(Services::Get<LayerManager>()->IsLayerInStack(&mLayer))
    {
        Services::Get<LayerManager>()->PopLayer();
    }
}

void Scene::Init()
{
    // After all models have been created, run through and execute init anims.
    // Want to wait until after creating all actors, in case init anims need to touch created actors!
    for(auto& modelDef : mSceneData->GetModels())
    {
        // Run any init anims specified.
        // These are usually needed to correctly position the model.
        if((modelDef->type == SceneModel::Type::Prop || modelDef->type == SceneModel::Type::GasProp)
           && modelDef->initAnim != nullptr)
        {
            //printf("Applying init anim for %s\n", modelDef->name.c_str());
            mAnimator->Sample(modelDef->initAnim, 0, modelDef->name);
        }
    }

    // Execute init anims for actors too.
    for(auto& actorDef : mSceneData->GetActors())
    {
        if(actorDef->initAnim != nullptr)
        {
            mAnimator->Sample(actorDef->initAnim, 0);
        }
    }

    // Init all actors and props.
    for(auto& prop : mProps)
    {
        prop->Init(*mSceneData);
    }
    for(auto& actor : mActors)
    {
        actor->Init(*mSceneData);
    }

    // Create soundtrack player and get it playing!
    Actor* actor = new Actor();
    mSoundtrackPlayer = actor->AddComponent<SoundtrackPlayer>();

    const std::vector<Soundtrack*>& soundtracks = mSceneData->GetSoundtracks();
    for(auto& soundtrack : soundtracks)
    {
        mSoundtrackPlayer->Play(soundtrack);
    }

    // Check for and run "scene enter" actions.
    Services::Get<ActionManager>()->ExecuteAction("SCENE", "ENTER");
}

void Scene::Update(float deltaTime)
{
    //TEMP: for debug visualization of BSP ambient light sources.
    //mSceneData->GetBSP()->DebugDrawAmbientLights(mEgo->GetPosition());

    // Apply ambient light to actors.
    for(auto& actor : mActors)
    {
        // Use the "model position" rather than the "actor position" for more accurate lighting.
        // For example, in RC1, Buthane's actor position is way outside the map (dark color), but her model is near the van.
        Color32 ambientColor = mSceneData->GetBSP()->CalculateAmbientLightColor(actor->GetModelPosition());
        for(Material& material : actor->GetMeshRenderer()->GetMaterials())
        {
            material.SetColor("uAmbientColor", ambientColor);
        }
    }

    // Check whether Ego has tripped any triggers.
    if(mEgo != nullptr && !mSceneData->GetTriggers().empty())
    {
        // Triggers consist of Rects on the X/Z plane. So, get Ego's X/Z pos.
        Vector3 egoPos = mEgo->GetPosition();
        Vector2 egoXZPos(egoPos.x, egoPos.z);

        // See if ego is inside any trigger rect.
        for(auto& trigger : mSceneData->GetTriggers())
        {
            //Debug::DrawRectXZ(trigger->rect, GetFloorY(egoPos) + 10.0f, Color32::Green);
            if(trigger->rect.Contains(egoXZPos))
            {
                // If so, treat the label as a noun (e.g. GET_CLOSE) with hardcoded "WALK" verb.
                Services::Get<ActionManager>()->ExecuteAction(trigger->label, "WALK");
            }
        }
    }
}

bool Scene::InitEgoPosition(const std::string& positionName)
{
	if(mEgo == nullptr) { return false; }
    
	// Get position.
    const ScenePosition* position = GetPosition(positionName);
	if(position == nullptr) { return false; }
    
    // Set position and heading.
    mEgo->SetPosition(position->position);
    mEgo->SetHeading(position->heading);
    mEgo->SnapToFloor();
	
	// Should also set camera position/angle.
	// Output a warning if specified position has no camera though.
	if(position->cameraName.empty())
	{
		Services::GetReports()->Log("Warning", "No camera information is supplied in position '" + positionName + "'.");
		return true;
	}
	
	// Move the camera to desired position/angle.
	SetCameraPosition(position->cameraName);
	return true;
}

void Scene::SetCameraPosition(const std::string& cameraName)
{
	// Find camera or fail. Any *named* camera type is valid.
	const SceneCamera* camera = mSceneData->GetRoomCamera(cameraName);
	if(camera == nullptr)
	{
		camera = mSceneData->GetCinematicCamera(cameraName);
		if(camera == nullptr)
		{
			camera = mSceneData->GetDialogueCamera(cameraName);
		}
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

void Scene::SetCameraPositionForConversation(const std::string& conversationName, bool isInitial)
{
    // Get dialogue camera associated with this conversation, either initial or final.
    const DialogueSceneCamera* camera = isInitial ?
        mSceneData->GetInitialDialogueCameraForConversation(conversationName) :
        mSceneData->GetFinalDialogueCameraForConversation(conversationName);
    if(camera == nullptr) { return; }

    // Set camera if we found it.
    mCamera->SetPosition(camera->position);
    mCamera->SetAngle(camera->angle);
}

void Scene::GlideToCameraPosition(const std::string& cameraName, std::function<void()> finishCallback)
{
    // Find camera or fail. Any *named* camera type is valid.
    const SceneCamera* camera = mSceneData->GetRoomCamera(cameraName);
    if(camera == nullptr)
    {
        camera = mSceneData->GetCinematicCamera(cameraName);
        if(camera == nullptr)
        {
            camera = mSceneData->GetDialogueCamera(cameraName);
        }
    }

    // If couldn't find a camera with this name, error out!
    if(camera == nullptr)
    {
        Services::GetReports()->Log("Error", "Error: '" + cameraName + "' is not a valid room camera.");
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // Do the glide.
    mCamera->Glide(camera->position, camera->angle, finishCallback);
}

SceneCastResult Scene::Raycast(const Ray& ray, bool interactiveOnly, const GKObject* ignore) const
{
	SceneCastResult result;
	
	// Check props/actors before BSP.
	// Later, we'll check BSP and see if we hit something obscuring a prop/actor.
    for(auto& object : mPropsAndActors)
    {
        // Ignore if desired.
        if(ignore != nullptr && ignore == object) { continue; }

        // If only interested in interactive objects, skip non-interactive objects.
        if(interactiveOnly && !object->CanInteract()) { continue; }

        // Raycast to see if this is the closest thing we've hit.
        // If so, we save it as our current result.
        MeshRenderer* meshRenderer = object->GetMeshRenderer();
        RaycastHit hitInfo;
        if(meshRenderer != nullptr && meshRenderer->Raycast(ray, hitInfo))
        {
            //Vector3 worldPos = ray.GetPoint(hitInfo.t);
            //printf("Raycast hit %s, yPos=%f\n", object->GetNoun().c_str(), worldPos.y);
            if(hitInfo.t < result.hitInfo.t)
            {
                result.hitInfo = hitInfo;
                result.hitObject = object;
            }
        }
    }
	
	// Assign name in hit info, if anything was hit.
	// This is because GKObjects don't currently do this during raycast.
	if(result.hitObject != nullptr)
	{
        //Debug::DrawSphere(Sphere(ray.GetPoint(result.hitInfo.t), 1.0f), Color32::Red, 1.0f);
		result.hitInfo.name = result.hitObject->GetNoun();
	}
	
	// Check BSP for any hit interactable object.
	BSP* bsp = mSceneData->GetBSP();
	if(bsp != nullptr)
	{
		RaycastHit hitInfo;
		if(bsp->RaycastNearest(ray, hitInfo))
		{
			// If "t" is smaller, then the BSP object obscured any previous hit.
			if(hitInfo.t < result.hitInfo.t)
			{
				result.hitInfo = hitInfo;
				
				// See if hit any actor representing BSP object.
				for(auto& bspActor : mBSPActors)
				{
                    // Ignore if desired.
                    if(ignore != nullptr && ignore == bspActor) { continue; }
                    
					// If only interested in interactive objects, skip non-interactive objects.
					if(interactiveOnly && !bspActor->CanInteract()) { continue; }
					
					// Check whether this is the BSP actor we hit.
					if(StringUtil::EqualsIgnoreCase(bspActor->GetName(), hitInfo.name))
					{
						result.hitObject = bspActor;
						break;
					}
				}
			}
		}
	}
	return result;
}

void Scene::Interact(const Ray& ray, GKObject* interactHint)
{
	// Ignore scene interaction while the action bar is showing.
	if(Services::Get<ActionManager>()->IsActionBarShowing()) { return; }
	
	// Also ignore scene interaction when inventory is up.
	if(Services::Get<InventoryManager>()->IsInventoryShowing()) { return; }
	
	// Get interacted object.
	GKObject* interacted = interactHint;
	if(interacted == nullptr)
	{
		SceneCastResult result = Raycast(ray, true);
		interacted = result.hitObject;
	}
	
	// If interacted object is null, see if we hit the floor, in which case we want to walk.
	if(interacted == nullptr)
	{
		BSP* bsp = mSceneData->GetBSP();
		if(bsp != nullptr)
		{
			// Cast ray against scene BSP to see if it intersects with anything.
			// If so, it means we clicked on that thing.
			RaycastHit hitInfo;
			if(bsp->RaycastNearest(ray, hitInfo))
			{
				// Clicked on the floor - move ego to position.
				if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData->GetFloorModelName()))
				{
					// Check walker boundary to see whether we can walk to this spot.
					mEgo->WalkTo(ray.GetPoint(hitInfo.t), nullptr);
				}
			}
		}
		return;
	}

    // Looks like we're interacting with something interesting.
    mActiveObject = interacted;
	
	// We've got an object to interact with!
	// See if it has a pre-defined verb with an associated action. If so, we will immediately execute that action (w/o showing action bar).
	if(!interacted->GetVerb().empty())
	{
        const Action* action = Services::Get<ActionManager>()->GetAction(interacted->GetNoun(), interacted->GetVerb());
        if(action != nullptr)
        {
            ExecuteAction(action);
            return;
        }
	}
	
	// No pre-defined verb OR no action for that noun/verb combo - try to show action bar.
	Services::Get<ActionManager>()->ShowActionBar(interacted->GetNoun(), std::bind(&Scene::ExecuteAction, this, std::placeholders::_1));
    ActionBar* actionBar = Services::Get<ActionManager>()->GetActionBar();
  
    // Add INSPECT/UNINSPECT if not present.
    bool alreadyInspecting = StringUtil::EqualsIgnoreCase(interacted->GetNoun(), mCamera->GetInspectNoun());
    if(alreadyInspecting)
    {
        if(!actionBar->HasVerb("INSPECT_UNDO"))
        {
            actionBar->AddVerbToFront("INSPECT_UNDO", [interacted](){
                Services::Get<ActionManager>()->ExecuteCustomAction(interacted->GetNoun(), "INSPECT_UNDO", "ALL", "wait UnInspect()");
            });
        }
    }
    else
    {
        if(!actionBar->HasVerb("INSPECT"))
        {
            actionBar->AddVerbToFront("INSPECT", [interacted](){
                Services::Get<ActionManager>()->ExecuteCustomAction(interacted->GetNoun(), "INSPECT", "ALL", "wait InspectObject()");
            });
        }
    }
}

void Scene::SkipCurrentAction()
{
    // If an action is playing, this should skip the action.
    if(Services::Get<ActionManager>()->IsActionPlaying())
    {
        Services::Get<ActionManager>()->SkipCurrentAction();
        return;
    }

    // If no action, but ego is walking, this can skip the walk.
    if(mEgo != nullptr && mEgo->GetWalker()->IsWalking())
    {
        mEgo->GetWalker()->SkipToEnd();
    }
}

BSPActor* Scene::GetHitTestObjectByModelName(const std::string& modelName) const
{
    for(auto& object : mBSPActors)
    {
        if(StringUtil::EqualsIgnoreCase(object->GetName(), modelName))
        {
            return object;
        }
    }
    return nullptr;
}

GKObject* Scene::GetSceneObjectByModelName(const std::string& modelName) const
{
	for(auto& object : mPropsAndActors)
	{
        if(StringUtil::EqualsIgnoreCase(object->GetName(), modelName))
        {
            return object;
        }
	}
    //TODO: Should we also check BSP here?
	return nullptr;
}

GKObject* Scene::GetSceneObjectByNoun(const std::string& noun) const
{
    for(auto& object : mPropsAndActors)
    {
        if(StringUtil::EqualsIgnoreCase(object->GetNoun(), noun))
        {
            return object;
        }
    }
    for(auto& object : mBSPActors)
    {
        if(StringUtil::EqualsIgnoreCase(object->GetNoun(), noun))
        {
            return object;
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

float Scene::GetFloorY(const Vector3& position) const
{
    // Calculate ray origin using passed position, but really high in the air!
    Vector3 rayOrigin = position;
    rayOrigin.y = 10000.0f;

    // Create ray with origin high in the sky and pointing straight down.
    Ray downRay(rayOrigin, -Vector3::UnitY);

    // Raycast straight down and test against the floor BSP.
    // If we hit something, just use the Y hit position as the floor's Y.
    BSP* bsp = mSceneData->GetBSP();
    if(bsp != nullptr)
    {
        RaycastHit hitInfo;
        if(bsp->RaycastSingle(downRay, mSceneData->GetFloorModelName(), hitInfo))
        {
            return downRay.GetPoint(hitInfo.t).y;
        }
    }

    // If didn't hit floor, just return 0.
    // TODO: Maybe we should return a default based on the floor BSP's height?
    return 0.0f;
}

Texture* Scene::GetFloorTexture(const Vector3& position) const
{
    BSP* bsp = mSceneData->GetBSP();
    if(bsp == nullptr) { return nullptr; }

    float height = 0.0f;
    Texture* texture = nullptr;
    bsp->GetFloorInfo(position, height, texture);
    return texture;
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

void Scene::SetPaused(bool paused)
{
    // Pause/unpause sound track player.
    if(mSoundtrackPlayer != nullptr)
    {
        mSoundtrackPlayer->SetEnabled(!paused);
    }
    
    // Pause/unpause animator.
    //TODO: You might expect that pausing the animator is necessary to effectively pause the scene.
    //TODO: However, doing so can break actions in inventory, since voice over actions rely on the Animator.
    //TODO: Not pausing animator seems to work fine for now, but if it's a problem later on, maybe we need to have a separate animator for non-scene stuff or something.
    /*
    if(mAnimator != nullptr)
    {
        mAnimator->SetEnabled(!paused);
    }
    */
    
    // Tell camera if the scene is active or not.
    // We don't want to set camera inactive, because we still want it to render.
    // And we don't want to disable updates b/c camera object handles player inputs, even if scene is paused.
    if(mCamera != nullptr)
    {
        mCamera->SetSceneActive(!paused);
    }
    
    // Pause/unpause all objects in the scene by disabling updates.
    for(auto& object : mPropsAndActors)
    {
        object->SetUpdateEnabled(!paused);
    }
    
    // For GKActors, they "internally" contain a separate actor for the 3D mesh.
    // So, we've also got to set the time scale on that.
    //TODO: Probably could hide this by adding GKObject/GKActor functions for pause/unpause.
    for(auto& actor : mActors)
    {
        actor->GetMeshRenderer()->GetOwner()->SetUpdateEnabled(!paused);
    }
}

void Scene::InspectActiveObject(std::function<void()> finishCallback)
{
    // We need an active object, for one.
    if(mActiveObject == nullptr)
    {
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // Inspect that object.
    InspectObject(mActiveObject->GetNoun(), finishCallback);
}

void Scene::InspectObject(const std::string& noun, std::function<void()> finishCallback)
{
    // Try to get inspect camera for this noun.
    const SceneCamera* inspectCamera = mSceneData->GetInspectCamera(noun);

    // If that fails, there can also be an inspect camera associated with the *model* name for this noun.
    if(inspectCamera == nullptr)
    {
        // See if we can find the object associated with this noun.
        GKObject* sceneObject = GetSceneObjectByNoun(noun);
        if(sceneObject != nullptr)
        {
            // For BSP objects, the BSP object name is stored in the "Name" field.
            inspectCamera = mSceneData->GetInspectCamera(sceneObject->GetName());

            // If that didn't work, try with the model name in the mesh renderer as a last resort.
            if(inspectCamera == nullptr && sceneObject->GetMeshRenderer() != nullptr)
            {
                inspectCamera = mSceneData->GetInspectCamera(sceneObject->GetMeshRenderer()->GetModelName());
            }
        }
    }

    // After all that, if we have an inspect camera, use it!
    if(inspectCamera != nullptr)
    {
        mCamera->Inspect(noun, inspectCamera->position, inspectCamera->angle, finishCallback);
    }
    else // No inspect camera for this noun
    {
        // This does actually happen for dynamic objects (like any humanoid/walker).
        // So, we need to support this. Find an inspect position on-the-fly!
        bool foundActor = false;
        for(GKActor* actor : mActors)
        {
            if(StringUtil::EqualsIgnoreCase(actor->GetNoun(), noun))
            {
                // For starters, at head level, looking toward them sounds reasonable.
                Vector3 facing = actor->GetForward();
                Vector3 inspectPos = actor->GetHeadPosition() + facing * 50.0f;

                float yaw = Heading::FromDirection(-facing).ToRadians();
                Vector2 inspectAngle(yaw, 0.0f);

                //TODO: If the actor is facing a wall or something, it's pretty likely the camera will go out-of-bounds.
                //TODO: The original game seems to check this and move the camera around to a valid spot on the sides/back.
                
                // Use the calculated inspect position and angle.
                mCamera->Inspect(noun, inspectPos, inspectAngle, finishCallback);
                foundActor = true;
                break;
            }
        }

        // Worst case, there's no inspect camera for this thing, so let's give up!
        if(!foundActor && finishCallback != nullptr)
        {
            finishCallback();
        }
    }
}

void Scene::UninspectObject(std::function<void()> finishCallback)
{
    mCamera->Uninspect(finishCallback);
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
				//Debug::DrawLine(mEgo->GetPosition(), scenePos->position, Color32::Green, 60.0f);
				mEgo->WalkTo(scenePos->position, scenePos->heading, [this, action]() -> void {
					Services::Get<ActionManager>()->ExecuteAction(action);
				});
			}
			else
			{
				Services::Get<ActionManager>()->ExecuteAction(action);
			}
			break;
		}
		case Action::Approach::Anim: // Example use: R25 Open/Close Window, Open/Close Dresser, Open/Close Drawer
		{
			Animation* anim = Services::GetAssets()->LoadAnimation(action->target);
			if(anim != nullptr)
			{
				mEgo->WalkToAnimationStart(anim, [action]() -> void {
					Services::Get<ActionManager>()->ExecuteAction(action);
				});
			}
			else
			{
				Services::Get<ActionManager>()->ExecuteAction(action);
			}
			break;
		}
		case Action::Approach::Near: // Never used in GK3.
		{
			std::cout << "Executed NEAR approach type!" << std::endl;
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
		case Action::Approach::NearModel: // Example use: RC1 Bookstore Door, Hallway R25 Door
		{
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
		case Action::Approach::Region: // Only use: RC1 "No Vacancies" Sign
		{
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
		case Action::Approach::TurnTo: // Never used in GK3.
		{
			std::cout << "Executed TURNTO approach type!" << std::endl;
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
		case Action::Approach::TurnToModel: // Example use: R25 Couch Sit, most B25
		{
			// Find position of the model.
			Vector3 modelPosition;
			GKObject* obj = GetSceneObjectByModelName(action->target);
			if(obj != nullptr)
			{
				modelPosition = obj->GetPosition();
			}
			else
			{
				modelPosition = mSceneData->GetBSP()->GetPosition(action->target);
			}
			
			// Get vector from Ego to model.
			//Debug::DrawLine(mEgo->GetPosition(), modelPosition, Color32::Green, 60.0f);
			Vector3 egoToModel = modelPosition - mEgo->GetPosition();
			
			// Do a "turn to" heading.
			Heading turnToHeading = Heading::FromDirection(egoToModel);
			mEgo->TurnTo(turnToHeading, [this, action]() -> void {
				Services::Get<ActionManager>()->ExecuteAction(action);
			});
			break;
		}
		case Action::Approach::WalkToSee: // Example use: R25 Look Painting/Couch/Dresser/Plant, RC1 Look Bench/Bookstore Sign
		{
            // The target could be a scene model, or it could be BSP. Figure it out!
            GKObject* obj = GetSceneObjectByModelName(action->target);
            if(obj == nullptr)
            {
                for(auto bspActor : mBSPActors)
                {
                    if(StringUtil::EqualsIgnoreCase(bspActor->GetName(), action->target))
                    {
                        obj = bspActor;
                        break;
                    }
                }
            }

            // If we found the object, walk to see it.
            // If didn't find it, print a warning/error and just execute right away.
            if(obj != nullptr)
            {
                mEgo->WalkToSee(obj, [this, action]() -> void{
                    Services::Get<ActionManager>()->ExecuteAction(action);
                });
            }
            else
            {
                std::cout << "Could not find WalkToSee target " << action->target << std::endl;
                Services::Get<ActionManager>()->ExecuteAction(action);
            }
			break;
		}
		case Action::Approach::None:
		{
			// Just do it!
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
		default:
		{
			Services::GetReports()->Log("Error", "Invalid approach " + std::to_string(static_cast<int>(action->approach)));
			Services::Get<ActionManager>()->ExecuteAction(action);
			break;
		}
	}
}
