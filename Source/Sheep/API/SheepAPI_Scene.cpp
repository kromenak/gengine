#include "SheepAPI_Scene.h"

#include "BSPActor.h"
#include "GEngine.h"
#include "GKActor.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "SceneData.h"
#include "StringUtil.h"
#include "WalkerBoundary.h"

using namespace std;

/*
shpvoid CallSceneFunction(std::string parameter)
{
    std::cout << "CallSceneFunction" << std::endl;
    return 0;
}
RegFunc1(CallSceneFunction, void, string, WAITABLE, REL_FUNC);
*/

int IsCurrentEgo(const std::string& actorName)
{
    const std::string& egoName = Scene::GetEgoName();
    return StringUtil::EqualsIgnoreCase(egoName, actorName) ? 1 : 0;
}
RegFunc1(IsCurrentEgo, int, string, IMMEDIATE, REL_FUNC);

std::string GetEgoName()
{
    return Scene::GetEgoName();
}
RegFunc0(GetEgoName, string, IMMEDIATE, REL_FUNC);

shpvoid InitEgoPosition(const std::string& positionName)
{
    if(!GEngine::Instance()->GetScene()->InitEgoPosition(positionName))
    {
        ExecError();
    }
    return 0;
}
RegFunc1(InitEgoPosition, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorPosition(const std::string& actorName, const std::string& positionName)
{
    // Get needed data.
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);

    // If either is null, log an error.
    if(actor == nullptr || scenePosition == nullptr)
    {
        ExecError();
        return 0;
    }

    // Docs are unclear about this, but in GK3, this definitely also sets heading.
    actor->SetPosition(scenePosition->position);
    actor->SetHeading(scenePosition->heading);
    return 0;
}
RegFunc2(SetActorPosition, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid DumpActorPosition(const std::string& actorName)
{
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->DumpPosition();
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc1(DumpActorPosition, void, string, IMMEDIATE, DEV_FUNC);

int IsActorNear(const std::string& actorName, const std::string& positionName, float distance)
{
    // Make sure distance is valid.
    if(distance < 0.0f)
    {
        Services::GetReports()->Log("Warning", StringUtil::Format("Warning: distance of %f is not valid - must be >= 0.", distance));
        ExecError();
        return 0;
    }

    // Get actor and position, or fail.
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
        if(scenePosition != nullptr)
        {
            // Distance check.
            return (actor->GetPosition() - scenePosition->position).GetLengthSq() < distance * distance;
        }
    }

    // Something was null...
    ExecError();
    return 0;
}
RegFunc3(IsActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);

int IsWalkingActorNear(const std::string& actorName, const std::string& positionName, float distance)
{
    // Make sure distance is valid.
    if(distance < 0.0f)
    {
        Services::GetReports()->Log("Warning", StringUtil::Format("Warning: distance of %f is not valid - must be >= 0.", distance));
        ExecError();
        return 0;
    }

    // Get actor and position, or fail.
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
        if(scenePosition != nullptr)
        {
            return (actor->GetWalkDestination() - scenePosition->position).GetLengthSq() < distance * distance;
        }
    }

    // Something was null...
    ExecError();
    return 0;
}
RegFunc3(IsWalkingActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);

/*
std::string GetIndexedPosition(int index)
{
    std::cout << "GetIndexedPosition" << std::endl;
    return "";
}
RegFunc1(GetIndexedPosition, string, int, IMMEDIATE, DEV_FUNC);

int GetPositionCount()
{
    std::cout << "GetPositionCount" << std::endl;
    return 0;
}
RegFunc0(GetPositionCount, int, IMMEDIATE, DEV_FUNC);
*/

int DoesModelExist(const std::string& modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
    return object != nullptr ? 1 : 0;
}
RegFunc1(DoesModelExist, int, string, IMMEDIATE, REL_FUNC);

int IsModelVisible(const std::string& modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        return object->IsActive() ? 1 : 0;
    }
    return 0;
}
RegFunc1(IsModelVisible, int, string, IMMEDIATE, REL_FUNC);

shpvoid ShowModel(const std::string& modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        object->SetActive(true);
    }
    return 0;
}
RegFunc1(ShowModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideModel(const std::string& modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        object->SetActive(false);
    }
    return 0;
}
RegFunc1(HideModel, void, string, IMMEDIATE, REL_FUNC);

int DoesSceneModelExist(const std::string& modelName)
{
    return GEngine::Instance()->GetScene()->DoesSceneModelExist(modelName) ? 1 : 0;
}
RegFunc1(DoesSceneModelExist, int, string, IMMEDIATE, REL_FUNC);

int IsSceneModelVisible(const std::string& modelName)
{
    return GEngine::Instance()->GetScene()->IsSceneModelVisible(modelName) ? 1 : 0;
}
RegFunc1(IsSceneModelVisible, int, string, IMMEDIATE, REL_FUNC);

shpvoid ShowSceneModel(const std::string& modelName)
{
    GEngine::Instance()->GetScene()->SetSceneModelVisibility(modelName, true);
    return 0;
}
RegFunc1(ShowSceneModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideSceneModel(const std::string& modelName)
{
    GEngine::Instance()->GetScene()->SetSceneModelVisibility(modelName, false);
    return 0;
}
RegFunc1(HideSceneModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid EnableHitTestModel(const std::string& modelName)
{
    BSPActor* hitTestActor = GEngine::Instance()->GetScene()->GetHitTestObjectByModelName(modelName);
    if(hitTestActor != nullptr)
    {
        hitTestActor->SetInteractive(true);
    }
    return 0;
}
RegFunc1(EnableHitTestModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid DisableHitTestModel(const std::string& modelName)
{
    BSPActor* hitTestActor = GEngine::Instance()->GetScene()->GetHitTestObjectByModelName(modelName);
    if(hitTestActor != nullptr)
    {
        hitTestActor->SetInteractive(false);
    }
    return 0;
}
RegFunc1(DisableHitTestModel, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid ShowModelGroup(std::string groupName)
{
    std::cout << "ShowModelGroup" << std::endl;
    return 0;
}
RegFunc1(ShowModelGroup, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideModelGroup(std::string groupName)
{
    std::cout << "HideModelGroup" << std::endl;
    return 0;
}
RegFunc1(HideModelGroup, void, string, IMMEDIATE, REL_FUNC);
*/

shpvoid WalkerBoundaryBlockModel(const std::string& modelName)
{
    // Find the object/model or fail.
    GKObject* obj = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
    if(obj == nullptr || obj->GetMeshRenderer() == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get the AABB for the object, as this is the area that's blocked.
    AABB modelAABB = obj->GetMeshRenderer()->GetAABB();
    Vector3 min = modelAABB.GetMin();
    Vector3 max = modelAABB.GetMax();

    // We need to convert to a Rect, as the walker boundary is flat.
    // So, just get rid of the Y-component.
    Rect worldRect(Vector2(min.x, min.z), Vector2(max.x, max.z));
    GEngine::Instance()->GetScene()->GetSceneData()->GetWalkerBoundary()->SetUnwalkableRect(modelName, worldRect);
    return 0;
}
RegFunc1(WalkerBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockModel(const std::string& modelName)
{
    GEngine::Instance()->GetScene()->GetSceneData()->GetWalkerBoundary()->ClearUnwalkableRect(modelName);
    return 0;
}
RegFunc1(WalkerBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex)
{
    //printf("WalkerBoundaryBlockRegion(%i, %i)\n", regionIndex, regionBoundaryIndex);
    GEngine::Instance()->GetScene()->GetSceneData()->GetWalkerBoundary()->SetRegionBlocked(regionIndex, regionBoundaryIndex, true);
    return 0;
}
RegFunc2(WalkerBoundaryBlockRegion, void, int, int, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockRegion(int regionIndex, int regionBoundaryIndex)
{
    GEngine::Instance()->GetScene()->GetSceneData()->GetWalkerBoundary()->SetRegionBlocked(regionIndex, regionBoundaryIndex, false);
    return 0;
}
RegFunc2(WalkerBoundaryUnblockRegion, void, int, int, IMMEDIATE, REL_FUNC);

/*
shpvoid WalkNear(std::string actorName, std::string positionName)
{
    std::cout << "WalkNear" << std::endl;
    return 0;
}
RegFunc2(WalkNear, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkNearModel(std::string actorName, std::string modelName)
{
    std::cout << "WalkNearModel" << std::endl;
    return 0;
}
RegFunc2(WalkNearModel, void, string, string, WAITABLE, REL_FUNC);
*/

shpvoid WalkTo(const std::string& actorName, const std::string& positionName)
{
    // If not in a scene, we'll just ignore walk to request.
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr)
    {
        return 0;
    }

    // Get actor.
    GKActor* actor = scene->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get position.
    const ScenePosition* scenePosition = scene->GetPosition(positionName);
    if(scenePosition == nullptr)
    {
        ExecError();
        return 0;
    }

    // Ok, we can actually do the walk to it seems!
    actor->WalkTo(scenePosition->position, scenePosition->heading, AddWait());
    return 0;
}
RegFunc2(WalkTo, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkToAnimation(const std::string& actorName, const std::string& animationName)
{
    // If not in a scene, we'll just ignore walk to request.
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr)
    {
        return 0;
    }

    // Get actor.
    GKActor* actor = scene->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get the animation.
    Animation* anim = Services::GetAssets()->LoadAnimation(animationName);
    if(anim == nullptr)
    {
        ExecError();
        return 0;
    }

    // Ok, we can actually do the walk to it seems!
    actor->WalkToAnimationStart(anim, AddWait());
    return 0;
}
RegFunc2(WalkToAnimation, void, string, string, WAITABLE, REL_FUNC);

/*
shpvoid WalkToSeeModel(std::string actorName, std::string modelName)
{
    std::cout << "WalkToSeeModel" << std::endl;
    return 0;
}
RegFunc2(WalkToSeeModel, void, string, string, WAITABLE, REL_FUNC);

//WalkToXZ
*/
