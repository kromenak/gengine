#include "Scene.h"

#include <iostream>
#include <limits>

#include "ActionBar.h"
#include "ActionManager.h"
#include "Animator.h"
#include "AssetManager.h"
#include "Billboard.h"
#include "BSPActor.h"
#include "Camera.h"
#include "Collisions.h"
#include "Color32.h"
#include "Debug.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GameTimers.h"
#include "GK3UI.h"
#include "GKActor.h"
#include "GKProp.h"
#include "GPSOverlay.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "PersistState.h"
#include "Profiler.h"
#include "RaycastTool.h"
#include "Renderer.h"
#include "ReportManager.h"
#include "SceneFunctions.h"
#include "SceneManager.h"
#include "Skybox.h"
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

/*static*/ Animator* Scene::GetGlobalAnimator()
{
    // Create and return a global animator that persists between scenes.
    static Animator* globalAnimator = nullptr;
    if(globalAnimator == nullptr)
    {
        Actor* globalAnimatorActor = new Actor("Global Animator");
        globalAnimatorActor->SetIsDestroyOnLoad(false);
        globalAnimator = globalAnimatorActor->AddComponent<Animator>();
    }
    return globalAnimator;
}

/*static*/ Animator* Scene::GetActiveAnimator()
{
    // Use the scene animator, if it's active.
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        Animator* sceneAnimator = scene->GetAnimator();
        if(sceneAnimator != nullptr && sceneAnimator->IsActiveAndEnabled())
        {
            return sceneAnimator;
        }
    }

    // Otherwise, fall back on global animator.
    return GetGlobalAnimator();
}

Scene::Scene(const std::string& name) :
    mLocation(name),
    mTimeblock(gGameProgress.GetTimeblock())
{
    mLayer.SetScene(this);

    // Create game camera.
    mCamera = new GameCamera();

    // Create animation player.
    Actor* animationActor = new Actor("Animator");
    mAnimator = animationActor->AddComponent<Animator>();

    // Scene layer is now active!
    gLayerManager.PushLayer(&mLayer);
}

Scene::~Scene()
{
    // Remove scene layer from stack (as well as all layers above it).
    while(gLayerManager.IsLayerInStack(&mLayer))
    {
        gLayerManager.PopLayer();
    }

    // Unload the scene if it wasn't done manually before deleting the scene.
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

    // Set location.
    gLocationManager.SetLocation(mLocation);

    // Creating scene data loads SIFs, but does nothing else yet!
    mSceneData = new SceneData(mLocation, mTimeblock.ToString());

    // It's generally important that we know how our "ego" will be as soon as possible.
    // This is because the scene loading *itself* may check who ego is to do certain things!
    mEgoSceneActor = mSceneData->DetermineWhoEgoWillBe();
    if(mEgoSceneActor != nullptr)
    {
        mEgoName = mEgoSceneActor->noun;
    }

    // Based on location, timeblock, and game progress, resolve what data we will load into the current scene.
    // Do this BEFORE incrementing location count, as SIF conditions sometimes do "zero-checks" (e.g. if current location count == 0).
    // After this, SceneData will have combined all SIFs and parsed all conditions to determine exactly what actors/models/etc to used right now.
    mSceneData->ResolveSceneData();

    // Create actors for the scene.
    const std::vector<const SceneActor*>& sceneActorDatas = mSceneData->GetActors();
    for(auto& actorDef : sceneActorDatas)
    {
        // NEVER spawn an ego who is not our current ego!
        if(actorDef->ego && actorDef != mEgoSceneActor) { continue; }
        CreateSceneActor(actorDef);
    }

    // Iterate over scene model data and prep the scene.
    // First, we want to hide any scene models that are set to "hidden".
    // Second, we want to spawn any non-scene models.
    const std::vector<const SceneModel*>& sceneModelDatas = mSceneData->GetModels();
    for(auto& modelDef : sceneModelDatas)
    {
        CreateSceneModel(modelDef);
    }

    // Create actors to represent positions in the scene.
    // This is just for debug purposes - no use in the final game.
    const std::vector<const ScenePosition*>& scenePositions = mSceneData->GetScenePositions();
    for(auto& scenePosition : scenePositions)
    {
        Actor* actor = new Actor(scenePosition->label);
        actor->SetPosition(scenePosition->position);
        actor->SetRotation(scenePosition->heading.ToQuaternion());
    }
}

void Scene::Unload()
{
    // On scene unload (or maybe de-init?), stop all soundtracks that were defined in the scene data.
    // A "force" flag is used here to indicate that any soundtrack that started on scene init should stop here, even if stop mode is "PlayToEnd".
    // This is required in at least one spot (KITCHEN soundtrack) - not sure if that was intentional on the original developers' part.
    const std::vector<Soundtrack*>& soundtracks = mSceneData->GetSoundtracks();
    for(auto& soundtrack : soundtracks)
    {
        mSoundtrackPlayer->Stop(soundtrack, true);
    }

    // Clear scene assets used in renderer.
    gRenderer.SetBSP(nullptr);
    gRenderer.SetSkybox(nullptr);

    // We're done with the scene data.
    delete mSceneData;
    mSceneData = nullptr;
}

void Scene::Init()
{
    // Init construction system.
    mConstruction.Init(this, mSceneData);

    // Create status overlay actor.
    // Do this after setting location so it shows the correct location!
    mStatusOverlay = new StatusOverlay();

    // Set BSP to be rendered.
    gRenderer.SetBSP(mSceneData->GetBSP());

    // Figure out if we have a skybox, and set it to be rendered.
    gRenderer.SetSkybox(mSceneData->GetSkybox());

    // Position the camera at the the default position and heading.
    const SceneCamera* defaultRoomCamera = mSceneData->GetDefaultRoomCamera();
    if(defaultRoomCamera != nullptr)
    {
        mCamera->SetPosition(defaultRoomCamera->position);
        mCamera->SetAngle(defaultRoomCamera->angle);
    }

    // Force a camera update to make sure the audio listener is positioned correctly in the scene.
    // This stops audio playing too loudly on scene load if the audio listener hasn't yet updated.
    mCamera->Update(0.0f);

    // If a camera bounds model exists for this scene, pass it along to the camera.
    for(auto& modelName : mSceneData->GetCameraBoundsModelNames())
    {
        Model* model = gAssetManager.LoadModel(modelName, AssetScope::Scene);
        if(model != nullptr)
        {
            mCamera->AddBounds(model);
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

    // Create soundtrack player and get it playing!
    Actor* actor = new Actor("SoundtrackPlayer");
    mSoundtrackPlayer = actor->AddComponent<SoundtrackPlayer>();
    for(auto& soundtrack : mSceneData->GetSoundtracks())
    {
        mSoundtrackPlayer->Play(soundtrack);
    }

    // If there's an "Init" SceneFunction for this Scene, execute it.
    SceneFunctions::Execute("Init");
}

void Scene::Enter()
{
    // Increment location counter after resolving scene data.
    // Despite SIFs sometimes doing "zero-checks," other NVC and SHP scripts typically do "one-checks".
    // E.g. run on "1st time enter" checks count == 1.
    gLocationManager.IncLocationCount(mEgoName, mLocation, mTimeblock);

    // Check for and run "scene enter" actions.
    gActionManager.ExecuteAction("SCENE", "ENTER");

    // This is a bit of a HACK, but we must update billboards after all has init'd, and scene enter functions have executed their first frames.
    // This is in case the camera position moves, and the billboard is facing the wrong way for one frame.
    for(auto& prop : mProps)
    {
        Model* model = prop->GetMeshRenderer()->GetModel();
        if(model != nullptr && model->IsBillboard())
        {
            Billboard* billboard = prop->GetComponent<Billboard>();
            billboard->LateUpdate(0.0f);
        }
    }

    // Make sure light colors are applied to all actors correctly before our first render.
    // Doing this here avoids a 1-frame render with bad lighting. Do this after SCENE/ENTER action executes, since that may change actor positions.
    ApplyAmbientLightColorToActors();
}

void Scene::Update(float deltaTime)
{
    RaycastTool::StartFrame();
    if(mPaused) { return; }

    //TEMP: for debug visualization of BSP ambient light sources.
    //GetBSP()->DebugDrawAmbientLights(mEgo->GetPosition());

    // Update ambient lighting on actors.
    ApplyAmbientLightColorToActors();

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
            if(trigger->rect.Contains(egoXZPos) && !gActionManager.IsActionPlaying())
            {
                // If so, treat the label as a noun (e.g. GET_CLOSE) with hardcoded "WALK" verb.
                gActionManager.ExecuteAction(trigger->label, "WALK");
            }
        }
    }

    // Update any game timers.
    GameTimers::Update(deltaTime);
}

bool Scene::InitEgoPosition(const std::string& positionName)
{
    if(mEgo == nullptr) { return false; }

    // Get position.
    const ScenePosition* position = GetPosition(positionName);
    if(position == nullptr) { return false; }

    // Set position and heading.
    mEgo->InitPosition(position);

    // Should also set camera position/angle.
    // Output a warning if specified position has no camera though.
    if(position->cameraName.empty())
    {
        gReportManager.Log("Warning", "No camera information is supplied in position '" + positionName + "'.");
        return true;
    }

    // Move the camera to desired position/angle.
    SetCameraPosition(position->cameraName);
    return true;
}

void Scene::SetCameraPosition(const std::string& cameraName)
{
    // Find camera or fail. Any *named* camera type is valid.
    const SceneCamera* camera = mSceneData->GetAnyCameraWithName(cameraName);

    // If couldn't find a camera with this name, error out!
    if(camera == nullptr)
    {
        gReportManager.Log("Error", "Error: '" + cameraName + "' is not a valid room camera.");
        return;
    }

    // Set position/angle.
    mCamera->SetPosition(camera->position);
    mCamera->SetAngle(camera->angle);
    mCamera->GetCamera()->SetCameraFovDegrees(camera->fov);
}

void Scene::SetCameraPositionForConversation(const std::string& conversationName, bool isInitial)
{
    // Get the dialogue camera associated with this conversation.
    // If this is initial (e.g. entering the conversation), try to use an initial camera, but fall back on final camera if initial camera doesn't exist (matches original game).
    const DialogueSceneCamera* camera = nullptr;
    if(isInitial)
    {
        camera = mSceneData->GetInitialDialogueCameraForConversation(conversationName);
    }
    if(camera == nullptr)
    {
        camera = mSceneData->GetFinalDialogueCameraForConversation(conversationName);
    }

    // Set camera if we found it.
    if(camera != nullptr)
    {
        mCamera->SetPosition(camera->position);
        mCamera->SetAngle(camera->angle);
        mCamera->GetCamera()->SetCameraFovDegrees(camera->fov);
    }
}

void Scene::GlideToCameraPosition(const std::string& cameraName, std::function<void()> finishCallback)
{
    // Find camera or fail. Any *named* camera type is valid.
    const SceneCamera* camera = mSceneData->GetAnyCameraWithName(cameraName);

    // If couldn't find a camera with this name, error out!
    if(camera == nullptr)
    {
        gReportManager.Log("Error", "Error: '" + cameraName + "' is not a valid room camera.");
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // Do the glide.
    //TODO: Set FOV here or no?
    mCamera->Glide(camera->position, camera->angle, finishCallback);
}

namespace
{
    bool IsInIgnoreList(GKObject** ignoreList, int ignoreCount, GKObject* object)
    {
        if(ignoreList == nullptr) { return false; }
        for(int i = 0; i < ignoreCount; ++i)
        {
            if(ignoreList[i] == object)
            {
                return true;
            }
        }
        return false;
    }
}

SceneCastResult Scene::Raycast(const Ray& ray, bool interactiveOnly, GKObject** ignore, int ignoreCount) const
{
    // First, iterate all Props and Actors to find the closest one that was hit by the ray and meets our criteria (if any).
    SceneCastResult result;
    for(GKObject* object : mAllGKObjects)
    {
        // Ignore inactive objects.
        if(!object->IsActive()) { continue; }

        // Completely ignore anything in the ignore list. It doesn't exist to us.
        if(IsInIgnoreList(ignore, ignoreCount, object)) { continue; }

        // See if the ray hits this object's 3D model.
        RaycastHit hitInfo;
        MeshRenderer* meshRenderer = object->GetMeshRenderer();
        if(meshRenderer != nullptr && meshRenderer->Raycast(ray, hitInfo))
        {
            hitInfo.name = object->GetNoun();
            RaycastTool::LogRaycastHit(hitInfo);

            // We did hit the 3D model, but is it closer than anything else we've hit thus far?
            if(hitInfo.t < result.hitInfo.t)
            {
                // It is the closest thing we've hit so far! Update the hit info struct.
                //printf("Raycast hit prop/actor %s, noun=%s t=%f\n", object->GetName().c_str(), object->GetNoun().c_str(), hitInfo.t);
                result.hitInfo = hitInfo;

                // BUT what if the closest thing we hit is non-interactive, and we only care about interactive things?
                // In this case, the non-interactive thing OBSCURES any previous interactive thing. Null out the hit object pointer!
                if(interactiveOnly && !object->CanInteract())
                {
                    result.hitObject = nullptr;
                }
                else
                {
                    result.hitObject = object;
                }
            }
        }
    }

    // The raycast logic for Actors/Props doesn't automatically fill in the HitInfo name field.
    // So, if something was hit, do that now.
    if(result.hitObject != nullptr)
    {
        result.hitInfo.name = result.hitObject->GetNoun();
    }

    // AT THIS POINT: we may or may not have hit a Prop or Actor with our ray. But now we need to see if any BSP geometry is closer.

    // Raycast against all BSP.
    RaycastHit bspHitInfo;
    if(GetBSP()->RaycastNearest(ray, bspHitInfo) && bspHitInfo.t < result.hitInfo.t)
    {
        // If we get here, some BSP is DEFINITELY closer to the screen than anything the ray has hit before.
        //printf("Raycast hit BSP %s, t=%f\n", bspHitInfo.name.c_str(), bspHitInfo.t);

        // Unless we find a valid BSP object below, we can start off assuming there is no hit object.
        // Even if there was a previously hit Prop/Actor, it must be obscured by BSP at this point.
        result.hitInfo = bspHitInfo;
        result.hitObject = nullptr;

        // Try to identify a BSPActor that correlates to the BSP geometry that was hit (and also meets our other criteria).
        for(BSPActor* object : mBSPActors)
        {
            // Name matches means that this BSPActor corresponds to the BSP geometry that was hit!
            if(StringUtil::EqualsIgnoreCase(object->GetName(), bspHitInfo.name))
            {
                //printf("Raycast hit BSPActor %s, t=%f\n", object->GetName().c_str(), bspHitInfo.t);

                // Completely ignore anything in the ignore list. It doesn't exist to us.
                if(IsInIgnoreList(ignore, ignoreCount, object)) { break; }

                // As long as this object meets the interactivity criteria, this BSPActor is our hit object.
                if(interactiveOnly && !object->CanInteract())
                {
                    result.hitObject = nullptr;
                }
                else
                {
                    result.hitObject = object;
                }
            }
        }
    }

    // In addition to 3D models and BSP, GK3 occasionally allows clicking on objects in the skybox.
    // So if we didn't hit anything else, check if we hit something interesting in the skybox.
    if(result.hitInfo.t == FLT_MAX)
    {
        Skybox* skybox = mSceneData->GetSkybox();
        if(skybox != nullptr)
        {
            // Skybox raycasts are unusual - the ray hits a mask texture, and the palette index at the hit pixel is returned.
            // In SIF files, the noun associated with that part of the skybox is mapped to that palette index.
            uint8_t paletteIndex = skybox->Raycast(ray);
            if(paletteIndex > 0)
            {
                auto it = mSkyboxHitTests.find(paletteIndex);
                if(it != mSkyboxHitTests.end())
                {
                    // We hit something in the skybox, and there is a noun associated with it - consider that a hit.
                    result.hitObject = it->second;
                    //printf("Sky hittest %s\n", result.hitObject->GetNoun().c_str());
                }
            }
        }
    }

    // For debugging, shows a red debug shape at the ray's closest intersection point.
    /*
    if(result.hitInfo.t < FLT_MAX)
    {
        Debug::DrawSphere(Sphere(ray.GetPoint(result.hitInfo.t - 0.1f), 1.0f), Color32::Red);
    }
    */
    return result;
}

SceneCastResult Scene::RaycastAABBs(const Ray& ray, GKObject** ignore, int ignoreCount) const
{
    SceneCastResult result;
    for(GKObject* object : mAllGKObjects)
    {
        // Ignore inactive objects.
        if(!object->IsActive()) { continue; }

        // Completely ignore anything in the ignore list. It doesn't exist to us.
        if(IsInIgnoreList(ignore, ignoreCount, object)) { continue; }

        // See if the ray hits this object's 3D model.
        float t;
        if(Intersect::TestRayAABB(ray, object->GetAABB(), t))
        {
            if(t < result.hitInfo.t)
            {
                result.hitObject = object;
                result.hitInfo.t = t;
            }
        }
    }

    if(result.hitObject != nullptr)
    {
        // If an object was hit, we should still check the BSP in general to see if some geometry is obscuring the object.
        RaycastHit bspHitInfo;
        if(GetBSP()->RaycastNearest(ray, bspHitInfo) && bspHitInfo.t < result.hitInfo.t)
        {
            result.hitObject = nullptr;
            result.hitInfo = bspHitInfo;
        }
        else
        {
            // If no BSP is in the way, fill in that we definitely hit an object with the raycast.
            result.hitInfo.name = result.hitObject->GetNoun();
            result.hitInfo.actor = result.hitObject;
        }
    }
    return result;
}

void Scene::Interact(const Ray& ray, GKObject* interactHint)
{
    // Ignore scene interaction while the action bar is showing.
    if(gActionManager.IsActionBarShowing()) { return; }

    // Also ignore scene interaction when inventory is up.
    if(gInventoryManager.IsInventoryShowing()) { return; }

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
        BSP* bsp = GetBSP();
        if(bsp != nullptr)
        {
            // Cast ray against scene BSP to see if it intersects with anything.
            // If so, it means we clicked on that thing.
            RaycastHit hitInfo;
            if(bsp->RaycastNearest(ray, hitInfo, true))
            {
                // Clicked on the floor - move ego to position.
                if(StringUtil::EqualsIgnoreCase(hitInfo.name, mSceneData->GetFloorModelName()))
                {
                    // 99% of the time, we want the normal walk behavior.
                    // In one scene (the final showdown with Montreaux) we need custom walk behavior of course.
                    if(mWalkOverrideCallback != nullptr)
                    {
                        mWalkOverrideCallback();
                    }
                    else
                    {
                        // When the player clicks somewhere on the floor, we try to walk to that spot.
                        // However, we don't want to allow walking to unwalkable spots. So first find the nearest walkable spot on the walker boundary.
                        Vector3 walkPos = mSceneData->GetWalkerBoundary()->FindNearestWalkablePosition(ray.GetPoint(hitInfo.t));

                        // Attempt to walk to the clicked position.
                        // It's 99% likely that a path exists to the walk pos, and the walk will succeed.
                        // The only edge case is if the walk pos clicked is on an isolated subsection of the walker boundary, in which case ego will walk as close as possible before giving up.
                        mEgo->WalkToBestEffort(walkPos, Heading::None, nullptr);
                    }
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
        const Action* action = gActionManager.GetAction(interacted->GetNoun(), interacted->GetVerb());
        if(action != nullptr)
        {
            ExecuteAction(action);
            return;
        }
    }

    // No pre-defined verb OR no action for that noun/verb combo - try to show action bar.
    gActionManager.ShowActionBar(interacted->GetNoun(), true, std::bind(&Scene::ExecuteAction, this, std::placeholders::_1));

    // Add INSPECT/UNINSPECT if not present.
    bool alreadyInspecting = StringUtil::EqualsIgnoreCase(interacted->GetNoun(), mCamera->GetInspectNoun());
    ActionBar* actionBar = gActionManager.GetActionBar();
    if(alreadyInspecting)
    {
        if(!actionBar->HasVerb("INSPECT_UNDO"))
        {
            actionBar->AddVerbToFront("INSPECT_UNDO", [interacted](){
                gActionManager.ExecuteCustomAction(interacted->GetNoun(), "INSPECT_UNDO", "ALL", "wait UnInspect()");
            });
            actionBar->SetVerbEnabled("INSPECT_UNDO", !mCamera->IsForcedCinematicMode());
        }

        // It's possible for INSPECT to be in the bar already, if it was a verb listed in NVC.
        // If so, we should remove it in this case - it's weird to have both INSPECT and INSPECT_UNDO options.
        actionBar->RemoveVerb("INSPECT");
    }
    else
    {
        if(!actionBar->HasVerb("INSPECT"))
        {
            actionBar->AddVerbToFront("INSPECT", [interacted](){
                gActionManager.ExecuteCustomAction(interacted->GetNoun(), "INSPECT", "ALL", "wait InspectObject()");
            });
            actionBar->SetVerbEnabled("INSPECT", !mCamera->IsForcedCinematicMode());
        }

        // Similar to above, we shouldn't have both INSPECT and INSPECT_UNDO active at the same time.
        actionBar->RemoveVerb("INSPECT_UNDO");
    }
}

void Scene::SkipCurrentAction()
{
    // If an action is playing, this should skip the action.
    if(gActionManager.IsActionPlaying() && !gActionManager.IsSkippingCurrentAction())
    {
        gActionManager.SkipCurrentAction();
        return;
    }

    // If no action, do walk skip. This skips all walking actors (ego or otherwise) to the end of their current walk.
    // Side note: for the longest time, I thought this only skips Ego's walk. But it actually skips ALL walker walks!
    // Even if Ego is not walking, and other characters are, their walks get skipped.
    for(GKActor* actor : mActors)
    {
        if(actor->GetWalker()->IsWalking())
        {
            actor->GetWalker()->SkipToEnd();
        }
    }
}

void Scene::SetEgo(const std::string& actorName)
{
    GKActor* actor = GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        mEgo = actor;
    }
}

GKObject* Scene::GetSceneObjectByModelName(const std::string& modelName) const
{
    for(auto& object : mAllGKObjects)
    {
        if(StringUtil::EqualsIgnoreCase(object->GetName(), modelName))
        {
            return object;
        }
    }
    return nullptr;
}

BSPActor* Scene::GetBSPActorByModelName(const std::string& modelName) const
{
    for(auto& bspActor : mBSPActors)
    {
        if(StringUtil::EqualsIgnoreCase(bspActor->GetName(), modelName))
        {
            return bspActor;
        }
    }
    return nullptr;
}

BSPActor* Scene::GetHitTestByModelName(const std::string& modelName) const
{
    for(auto& hitTest : mHitTests)
    {
        if(StringUtil::EqualsIgnoreCase(hitTest->GetName(), modelName))
        {
            return hitTest;
        }
    }
    return nullptr;
}

GKProp* Scene::GetPropByModelName(const std::string& modelName) const
{
    for(auto& prop : mProps)
    {
        if(StringUtil::EqualsIgnoreCase(prop->GetName(), modelName))
        {
            return prop;
        }
    }
    return nullptr;
}

GKObject* Scene::GetSceneObjectByNoun(const std::string& noun) const
{
    for(auto& object : mAllGKObjects)
    {
        if(StringUtil::EqualsIgnoreCase(object->GetNoun(), noun))
        {
            return object;
        }
    }
    return nullptr;
}

GKProp* Scene::GetPropByNoun(const std::string& noun) const
{
    for(auto& prop : mProps)
    {
        if(StringUtil::EqualsIgnoreCase(prop->GetNoun(), noun))
        {
            return prop;
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
    gReportManager.Log("Error", "Error: Who the hell is '" + noun + "'?");
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
        gReportManager.Log("Error", "Error: '" + positionName + "' is not a valid position. Call DumpPositions() to see valid positions.");
    }
    return position;
}

float Scene::GetFloorY(const Vector3& position) const
{
    if(GetBSP() == nullptr) { return 0.0f; }

    float height = 0.0f;
    Texture* texture = nullptr;
    GetBSP()->GetFloorInfo(position, height, texture);
    return height;
}

bool Scene::GetFloorInfo(const Vector3& position, float& outHeight, Texture*& outTexture)
{
    return GetBSP()->GetFloorInfo(position, outHeight, outTexture);
}

void Scene::ApplyTextureToSceneModel(const std::string& modelName, Texture* texture)
{
    GetBSP()->SetTexture(modelName, texture);
}

void Scene::SetSceneModelVisibility(const std::string& modelName, bool visible)
{
    // If there's a BSPActor for this model, set visibility through that API.
    BSPActor* bspActor = GetBSPActorByModelName(modelName);
    if(bspActor != nullptr)
    {
        // This function's effect is inconsistent between normal scene models and hit test models.
        if(bspActor->IsHitTest())
        {
            // For hit tests, visibility is disabled, but interactivity is unaffected (use EnableHitTest/DisableHitTest for that).
            bspActor->SetVisible(visible);
        }
        else
        {
            // For normal scene models, both visibility and interactivity are disabled. So we can set the whole active flag.
            bspActor->SetActive(visible);
        }
    }
    else
    {
        // No BSPActor, so just go directly to the BSP.
        GetBSP()->SetVisible(modelName, visible);
    }
}

bool Scene::IsSceneModelVisible(const std::string& modelName) const
{
    return GetBSP()->IsVisible(modelName);
}

bool Scene::DoesSceneModelExist(const std::string& modelName) const
{
    return GetBSP()->Exists(modelName);
}

void Scene::SetFixedModelLighting(const std::string& modelName, const Color32& color)
{
    GKObject* object = GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        object->SetFixedLightingColor(color);
    }
}

void Scene::SetPaused(bool paused)
{
    mPaused = paused;

    // Pause/unpause sound track player.
    if(mSoundtrackPlayer != nullptr)
    {
        mSoundtrackPlayer->SetEnabled(!paused);
    }

    // Pause/unpause animator.
    //TODO: You might expect that pausing the animator is necessary to effectively pause the scene.
    //TODO: However, doing so can break actions in inventory, since voice over actions rely on the Animator.
    //TODO: Not pausing animator seems to work fine for now, but if it's a problem later on, maybe we need to have a separate animator for non-scene stuff or something.
    if(mAnimator != nullptr)
    {
        mAnimator->SetEnabled(!paused);
    }

    // Tell camera if the scene is active or not.
    // We don't want to set camera inactive, because we still want it to render.
    // And we don't want to disable updates b/c camera object handles player inputs, even if scene is paused.
    if(mCamera != nullptr)
    {
        mCamera->SetSceneActive(!paused);
    }

    // Pause/unpause all objects in the scene by disabling updates.
    for(auto& object : mAllGKObjects)
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

void Scene::InspectActiveObject(const std::function<void()>& finishCallback)
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

void Scene::InspectObject(const std::string& noun, const std::function<void()>& finishCallback)
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
    InspectObject(noun, inspectCamera, finishCallback);
}

void Scene::InspectObject(const std::string& noun, const std::string& cameraName, const std::function<void()>& finishCallback)
{
    InspectObject(noun, mSceneData->GetAnyCameraWithName(cameraName), finishCallback);
}

void Scene::InspectObject(const std::string& noun, const SceneCamera* camera, const std::function<void()>& finishCallback)
{
    // After all that, if we have an inspect camera, use it!
    if(camera != nullptr)
    {
        mCamera->Inspect(noun, camera->position, camera->angle, finishCallback);
    }
    else // No inspect camera for this noun
    {
        // All moving objects (particularly humanoids/walkers) don't have a set inspect camera. But you can still inspect!
        // In these cases, the game finds a decent position/angle for the inspect camera on-the-fly.

        // First, find the object.
        GKObject* obj = GetSceneObjectByNoun(noun);
        if(obj == nullptr)
        {
            if(finishCallback != nullptr)
            {
                finishCallback();
            }
            return;
        }

        // We need to figure out the best inspect camera position and camera angle for this object.
        Vector3 inspectPosition;
        Vector2 inspectAngle;

        // For GKActors, the game tries to look at the character's face.
        const float kInspectPositionDistance = 50.0f;
        if(obj->IsA<GKActor>())
        {
            GKActor* actor = static_cast<GKActor*>(obj);

            // Get position X units in front of their head.
            Vector3 facing = actor->GetForward();
            inspectPosition = actor->GetHeadPosition() + facing * kInspectPositionDistance;

            // Set angle such that we're looking at the head.
            inspectAngle.x = Heading::FromDirection(-facing).ToRadians();
        }
        else
        {
            // Calculate the camera distance
            AABB aabb = obj->GetAABB();
            Vector3 aabbSize = aabb.GetSize();
            float largestSize = Math::Max(aabbSize.z, Math::Max(aabbSize.x, aabbSize.y));
            float distance = Math::Abs(largestSize / Math::Sin(mCamera->GetCamera()->GetCameraFovRadians()));

            // If the camera moved in a straight line from here to the object, find a point along that line to stop at.
            Vector3 objToCameraDir = Vector3::Normalize(mCamera->GetPosition() - aabb.GetCenter());
            inspectPosition = aabb.GetCenter() + objToCameraDir * distance;

            // Set angle such that we're looking at the thing.
            inspectAngle.x = Heading::FromDirection(-objToCameraDir).ToRadians();
            inspectAngle.y = Math::Asin(objToCameraDir.y);
        }

        // Move the camera to that position/angle to inspect.
        mCamera->Inspect(noun, inspectPosition, inspectAngle, finishCallback);
    }
}

void Scene::UninspectObject(const std::function<void()>& finishCallback)
{
    mCamera->Uninspect(finishCallback);
}

void Scene::OverrideSceneAsset(const std::string& sceneAssetName, const std::string& floorModelName)
{
    // Save the override scene asset name.
    mOverrideSceneName = sceneAssetName;

    // Load scene geometry based on asset name.
    SceneGeometryData geometryData;
    geometryData.Load(sceneAssetName);

    // If there's BSP, use it!
    mOverrideBSP = geometryData.GetBSP();
    if(mOverrideBSP != nullptr)
    {
        // Make sure floor model name is set correctly.
        // This is important so walkers know what to consider floor height, and for clicking to walk.
        if(!floorModelName.empty())
        {
            mOverrideBSP->SetFloorObjectName(floorModelName);
        }
        else
        {
            mOverrideBSP->SetFloorObjectName(mSceneData->GetFloorModelName());
        }

        // In some cases, the BSP is a different variant of the current scene, but with the same hit tests present (e.g. CEM in Day 1, 6PM).
        // In those cases, we need to re-enable the hit tests in the new scene, or else certain scene interactions may not work.
        for(BSPActor* hitTest : mHitTests)
        {
            mOverrideBSP->SetVisible(hitTest->GetName(), false);
            mOverrideBSP->SetHitTest(hitTest->GetName(), true);
        }

        // Use this BSP to render instead.
        gRenderer.SetBSP(mOverrideBSP);
    }
}

void Scene::ClearSceneAssetOverride()
{
    // Clear the overridden scene name and revert back to the original scene data's BSP.
    mOverrideSceneName.clear();
    mOverrideBSP = nullptr;
    gRenderer.SetBSP(mSceneData->GetBSP());
}

void Scene::OnPersist(PersistState& ps)
{
    // Scene data save/load was introduced in format version 4, so early out if this is older save data.
    if(ps.GetFormatVersionNumber() < 4) { return; }

    // DISCLAIMER & WARNING!
    // This save/load code is very different from the original game (see PERSISTENCE.DOC).
    // The original game used a very generalized save solution. I hoped to replicate it, but it was a very complex undertaking.
    // Maybe someday that can happen, but for now, this save code is much more targeted. And our requirements are different from the original devs:
    // 1) The original devs may have been developing a save system to be used by many future games. This one is only for GK3.
    // 2) The original devs needed a save system that worked with a game under active development. This one is only for a final product.

    // This code makes several assumptions:
    // 1) Scene::Load and Scene::Init have already completed for the current scene.
    // 2) Certain objects (camera, BSP, soundtrack player, etc) definitely exist in their initial states.

    // This code is not particularly resilient, though it does try to guard against objects not being present on load that were in the save file.
    // Over time, additional guards and error handling may need to be added.

    // Save/load the game camera. This includes camera position, pitch/yaw, and other state data.
    mCamera->OnPersist(ps);

    // Save/load playing soundtracks.
    // This is most important for scenes that change their music mid-way through the scene (Montreaux's office for example).
    mSoundtrackPlayer->OnPersist(ps);

    // Save/load BSP state.
    // Needed because BSP textures/visibility sometimes change during a scene.
    mSceneData->GetBSP()->OnPersist(ps);

    // Save/load any override scene name.
    ps.Xfer(PERSIST_VAR(mOverrideSceneName));

    // Need to save/load walker boundary to remember changes to walkable regions that may occur during a scene.
    mSceneData->GetWalkerBoundary()->OnPersist(ps);

    // We don't care about 99% of overlays because the game doesn't let you save during them.
    // One exception is the GPS. This ensures that the GPS is visible if you save when it is showing.
    GPSOverlay::OnPersist(ps);

    //TODO: Should we restore Sidney state, if you save inside Sidney? YES!

    // On rare occasions (Hexagram Puzzle), the scene Ego may change. So, we need to save this too.
    std::string egoNoun = mEgo != nullptr ? mEgo->GetNoun() : "";
    ps.Xfer(PERSIST_VAR(egoNoun));
    if(ps.IsLoading())
    {
        mEgo = GetActorByNoun(egoNoun);
    }

    // SAVE/LOAD GKOBJECTS
    // Get object count.
    uint64_t objectCount = mAllGKObjects.size();
    ps.Xfer(PERSIST_VAR(objectCount));

    // Iterate each scene object, one by one.
    for(uint64_t i = 0; i < objectCount; ++i)
    {
        // If we're saving, just save this actor's data out - easy!
        if(ps.IsSaving())
        {
            // Save the object name.
            std::string name = mAllGKObjects[i]->GetName();
            ps.Xfer(PERSIST_VAR(name));
            //printf("Save %i (%s)\n", static_cast<int>(i), name.c_str());

            // Save the object type.
            TypeId objType = mAllGKObjects[i]->GetTypeId();
            ps.Xfer(PERSIST_VAR(objType));

            // Before saving the object data, write out a placeholder int.
            // We'll come back later and fill this in with the object size.
            uint32_t objSizePos = ps.GetBinaryWriter()->GetPosition();
            ps.GetBinaryWriter()->WriteUInt(0);
            uint32_t objDataStartPos = ps.GetBinaryWriter()->GetPosition();

            // Save the object.
            mAllGKObjects[i]->OnPersist(ps);

            // Go back and fill in the object size.
            uint32_t objDataEndPos = ps.GetBinaryWriter()->GetPosition();
            ps.GetBinaryWriter()->Seek(objSizePos);
            ps.GetBinaryWriter()->WriteInt(objDataEndPos - objDataStartPos);
            ps.GetBinaryWriter()->Seek(objDataEndPos);
        }
        else
        {
            // Read the object name.
            std::string name;
            ps.Xfer(PERSIST_VAR(name));
            //printf("Load %i (%s)\n", static_cast<int>(i), name.c_str());

            // Read the object type.
            TypeId objType = INVALID_TYPE_ID;
            ps.Xfer(PERSIST_VAR(objType));

            // Read the object size.
            uint32_t objSize = ps.GetBinaryReader()->ReadUInt();

            // If loading, we need to find the object with the specified name first.
            GKObject* object = GetSceneObjectByModelName(name);

            // If the object exists, but the type is different than what was saved, we have to skip it.
            // This can happen sometimes, if an object switches from a dynamic prop to static BSP based on scene load conditions.
            if(object != nullptr && object->GetTypeId() != objType)
            {
                printf("Type mismatch for object %s - skipping.\n", name.c_str());
                ps.GetBinaryReader()->Skip(objSize);
            }
            else if(object == nullptr) // can't find the object - can maybe recover
            {
                // It's possible that this object was not loaded due to the current game state. A good example:
                // 1) When you first meet Mosely, the kitchen door is spawned as a dynamic prop to support the cutscene (and the bsp version is hidden).
                // 2) If you save, the dynamic kitchen door prop is in the save data, and the bsp version is hidden in the save data.
                // 3) On load, since you already met Mosely, the dynamic door prop is not loaded. But the BSP version is also hidden. No kitchen door appears!
                // To get around this, we should try to spawn objects present in the save data if we can't find them in the loaded scene.
                const SceneModel* sceneModel = mSceneData->FindSceneModelForLoad(name);
                if(sceneModel != nullptr)
                {
                    GKObject* newObj = CreateSceneModel(sceneModel);
                    newObj->OnPersist(ps);
                }
                else
                {
                    const SceneActor* sceneActor = mSceneData->FindSceneActorForLoad(name);
                    if(sceneActor != nullptr)
                    {
                        GKActor* newActor = CreateSceneActor(sceneActor);
                        newActor->OnPersist(ps);
                    }
                    else
                    {
                        printf("Object %s doesn't exist in the scene and can't be spawned - skipping.\n", name.c_str());
                        ps.GetBinaryReader()->Skip(objSize);
                    }
                }
            }
            else
            {
                // Load the object's state.
                object->OnPersist(ps);
            }
        }
    }

    // Occasionally, special scene objects need to also save/load their state. They'll add a callback if so.
    // Some examples: Church Angels, Montreaux Office Laser Heads, Chessboard, Pendulum, Invisible Bridge.
    for(auto& callback : mPersistCallbacks)
    {
        callback(ps);
    }

    // If loading, do a final step to set any override scene.
    // The overridden BSP inherits certain scene hittest object properties, so we want to do this AFTER all scene objects have been loaded.
    if(ps.IsLoading())
    {
        if(!mOverrideSceneName.empty())
        {
            OverrideSceneAsset(mOverrideSceneName);
        }
    }
}

GKObject* Scene::CreateSceneModel(const SceneModel* sceneModel)
{
    switch(sceneModel->type)
    {
        // "Scene" type models are ones that are baked into the BSP geometry.
        case SceneModel::Type::Scene:
        {
            BSPActor* actor = mSceneData->GetBSP()->CreateBSPActor(sceneModel->name);
            if(actor == nullptr) { break; }

            // This should be a member of all props/actors and BSP actors.
            mAllGKObjects.push_back(actor);
            mBSPActors.push_back(actor);

            // Set noun and verb based on model spec.
            actor->SetNoun(sceneModel->noun);
            actor->SetVerb(sceneModel->verb);

            // If it should be hidden by default, set inactive so that all visibility/interactivity is disabled.
            if(sceneModel->hidden)
            {
                actor->SetActive(false);
            }
            return actor;
        }

        // "HitTest" type models should be hidden, but still interactive.
        case SceneModel::Type::HitTest:
        {
            // If this is a skybox hit test, it requires special processing.
            if(StringUtil::StartsWithIgnoreCase(sceneModel->name, "skybox_"))
            {
                GKObject* obj = new GKObject();
                obj->SetNoun(sceneModel->noun);

                uint8_t paletteIndex = static_cast<uint8_t>(StringUtil::ToInt(sceneModel->name.substr(7)));
                mSkyboxHitTests[paletteIndex] = obj;
                return obj;
            }
            else // Otherwise, this hit test should be in the BSP.
            {
                BSPActor* actor = mSceneData->GetBSP()->CreateBSPActor(sceneModel->name);
                if(actor == nullptr) { break; }

                // This is a member of all props/actors, BSP actors, and hit test actors!
                mAllGKObjects.push_back(actor);
                mBSPActors.push_back(actor);
                mHitTests.push_back(actor);

                // Set noun/verb based on model spec.
                actor->SetNoun(sceneModel->noun);
                actor->SetVerb(sceneModel->verb);

                // Mark this object as a hit test (rather than a normal BSP-based scene model).
                actor->SetHitTest(true);

                // Hit test actors are never *visible* (unless you enable a debug option).
                actor->SetVisible(false);

                // However, if hidden flag is true, it also means the actor is not interactable.
                if(sceneModel->hidden)
                {
                    actor->SetInteractive(false);
                }
                return actor;
            }
        }

        // "Prop" and "GasProp" models both render their own model geometry.
        // Only difference for a "GasProp" is that it uses a provided Gas file too.
        case SceneModel::Type::Prop:
        case SceneModel::Type::GasProp:
        {
            GKProp* prop = new GKProp(*sceneModel);
            mAllGKObjects.push_back(prop);
            mProps.push_back(prop);
            return prop;
        }

        default:
            std::cout << "Unaccounted for model type: " << (int)sceneModel->type << std::endl;
            break;
    }
    return nullptr;
}

GKActor* Scene::CreateSceneActor(const SceneActor* sceneActor)
{
    // Create actor.
    GKActor* actor = new GKActor(sceneActor);
    mAllGKObjects.push_back(actor);
    mActors.push_back(actor);

    // If this is ego, save a reference to it.
    if(sceneActor->ego && sceneActor == mEgoSceneActor)
    {
        mEgo = actor;
    }

    // Create DOR prop, if one exists for this actor.
    // The DOR model assists with calculating an actor's facing direction, particularly while walking.
    Model* walkerDorModel = gAssetManager.LoadModel("DOR_" + actor->GetMeshRenderer()->GetModelName());
    if(walkerDorModel != nullptr)
    {
        GKProp* walkerDOR = new GKProp(walkerDorModel);
        actor->SetModelFacingHelper(walkerDOR);

        // Should this be in the mProps list? Going to say no for now...
        mAllGKObjects.push_back(walkerDOR);

        // Disable DOR mesh renderer so you can't see the placeholder model.
        walkerDOR->GetMeshRenderer()->SetEnabled(false);
    }
    return actor;
}

void Scene::ApplyAmbientLightColorToActors()
{
    // Apply ambient light to actors.
    for(GKActor* actor : mActors)
    {
        // Use the "model position" rather than the "actor position" for more accurate lighting.
        // For example, in RC1, Buthane's actor position is way outside the map (dark color), but her model is near the van.
        Color32 ambientColor = GetBSP()->CalculateAmbientLightColor(actor->GetFloorPosition());
        for(Material& material : actor->GetMeshRenderer()->GetMaterials())
        {
            material.SetColor("uAmbientColor", ambientColor);
        }
    }

    //TODO: Should we also do props here?
}

BSP* Scene::GetBSP() const
{
    if(mOverrideBSP != nullptr) { return mOverrideBSP; }
    return mSceneData != nullptr ? mSceneData->GetBSP() : nullptr;
}

void Scene::ExecuteAction(const Action* action)
{
    // Ignore nulls.
    if(action == nullptr) { return; }

    // Log to "Actions" stream.
    gReportManager.Log("Actions", "Playing NVC " + action->ToString());

    // Before executing the NVC, we need to handle any approach.
    switch(action->approach)
    {
        case Action::Approach::WalkTo:
        {
            const ScenePosition* scenePos = mSceneData->GetScenePosition(action->target);
            if(scenePos != nullptr)
            {
                //Debug::DrawLine(mEgo->GetPosition(), scenePos->position, Color32::Green, 60.0f);
                mEgo->WalkToExact(scenePos->position, scenePos->heading, [action]() {
                    if(!gActionManager.IsActionBarShowing())
                    {
                        gActionManager.ExecuteAction(action, nullptr, false);
                    }
                });
            }
            else
            {
                gActionManager.ExecuteAction(action, nullptr, false);
            }
            break;
        }
        case Action::Approach::Anim: // Example use: R25 Open/Close Window, Open/Close Dresser, Open/Close Drawer
        {
            Animation* anim = gAssetManager.LoadAnimation(action->target, AssetScope::Scene);
            if(anim != nullptr)
            {
                mEgo->WalkToAnimationStart(anim, [action]() {
                    if(!gActionManager.IsActionBarShowing())
                    {
                        gActionManager.ExecuteAction(action, nullptr, false);
                    }
                });
            }
            else
            {
                gActionManager.ExecuteAction(action, nullptr, false);
            }
            break;
        }
        case Action::Approach::Near: // Never used in GK3.
        {
            std::cout << "Executed NEAR approach type!" << std::endl;
            gActionManager.ExecuteAction(action, nullptr, false);
            break;
        }
        case Action::Approach::NearModel: // Example use: RC1 Bookstore Door, Hallway R25 Door
        {
            // Find the scene object from the model name.
            GKObject* obj = GetSceneObjectByModelName(action->target);
            if(obj != nullptr)
            {
                // Find nearest walkable pos to this object.
                Vector3 objPos = obj->GetAABB().GetCenter();
                Vector3 walkPos = mSceneData->GetWalkerBoundary()->FindNearestWalkablePosition(objPos);

                // We also want "turn to" behavior if already at the walk pos.
                Heading walkHeading = Heading::None;
                float distSq = (mEgo->GetPosition() - objPos).GetLengthSq();
                if(distSq < 60.0f * 60.0f)
                {
                    walkPos = mEgo->GetPosition();
                    walkHeading = Heading::FromDirection(objPos - mEgo->GetPosition());
                }

                // Walk there, and then do the action.
                mEgo->WalkToBestEffort(walkPos, walkHeading, [action](){
                    if(!gActionManager.IsActionBarShowing())
                    {
                        gActionManager.ExecuteAction(action, nullptr, false);
                    }
                });
            }
            else
            {
                // Just do the action if model could not be found.
                gActionManager.ExecuteAction(action, nullptr, false);
            }
            break;
        }
        case Action::Approach::Region: // Never used in GK3 (it does appear once in a SIF file, but it is misconfigured with an invalid region anyway).
        {
            gActionManager.ExecuteAction(action, nullptr, false);
            break;
        }
        case Action::Approach::TurnTo: // Never used in GK3.
        {
            gActionManager.ExecuteAction(action, nullptr, false);
            break;
        }
        case Action::Approach::TurnToModel: // Example use: R25 Couch Sit, most B25
        {
            // Find position of the model.
            Vector3 modelPosition;
            GKObject* obj = GetSceneObjectByModelName(action->target);
            if(obj != nullptr)
            {
                // The AABB center seems to give better results than just "GetPosition".
                // This is because some models have their visuals displaced from the actual position (coffee cups in Dining Room for example).
                modelPosition = obj->GetAABB().GetCenter();
            }
            else
            {
                // BSP that had a BSPActor generated will actually still use the above AABB code.
                // Worst case though, if we have some target that only exists in BSP, get its position directly.
                modelPosition = GetBSP()->GetPosition(action->target);
            }

            // Get vector from Ego to model.
            //Debug::DrawLine(mEgo->GetPosition(), modelPosition, Color32::Green, 60.0f);
            Vector3 egoToModel = modelPosition - mEgo->GetPosition();

            // Do a "turn to" heading.
            Heading turnToHeading = Heading::FromDirection(egoToModel);
            mEgo->TurnTo(turnToHeading, [action]() -> void {
                if(!gActionManager.IsActionBarShowing())
                {
                    gActionManager.ExecuteAction(action, nullptr, false);
                }
            });
            break;
        }
        case Action::Approach::WalkToSee: // Example use: R25 Look Painting/Couch/Dresser/Plant, RC1 Look Bench/Bookstore Sign
        {
            // Find the target object by name.
            GKObject* obj = GetSceneObjectByModelName(action->target);

            // If we found the object, walk to see it.
            // If didn't find it, print a warning/error and just execute right away.
            if(obj != nullptr)
            {
                mEgo->WalkToSee(obj, [action]() {
                    if(!gActionManager.IsActionBarShowing())
                    {
                        gActionManager.ExecuteAction(action, nullptr, false);
                    }
                });
            }
            else
            {
                std::cout << "Could not find WalkToSee target " << action->target << std::endl;
                gActionManager.ExecuteAction(action, nullptr, false);
            }
            break;
        }
        case Action::Approach::None:
        {
            // Just do it!
            gActionManager.ExecuteAction(action, nullptr, false);
            break;
        }
        default:
        {
            gReportManager.Log("Error", "Invalid approach " + std::to_string(static_cast<int>(action->approach)));
            gActionManager.ExecuteAction(action, nullptr, false);
            break;
        }
    }
}