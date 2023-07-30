#include "SheepAPI_Camera.h"

#include "Camera.h"
#include "GameCamera.h"
#include "GKObject.h"
#include "MeshRenderer.h"
#include "ReportManager.h"
#include "SaveManager.h"
#include "SceneManager.h"

using namespace std;

namespace
{
    Camera* GetCamera()
    {
        Scene* scene = gSceneManager.GetScene();
        if(scene != nullptr)
        {
            GameCamera* gameCamera = gSceneManager.GetScene()->GetCamera();
            if(gameCamera != nullptr)
            {
                return gameCamera->GetCamera();
            }
        }
        return nullptr;
    }
}

float GetCameraFOV()
{
    // Dig down to grab the value.
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        return camera->GetCameraFovDegrees();
    }
    return 0.0f;
}
RegFunc0(GetCameraFOV, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraFOV(float fov)
{
    // Clamp argument in valid range.
    if(fov < 1.0f || fov > 180.0f)
    {
        gReportManager.Log("Warning", "Warning: camera FOV must be between 1 and 180. Clamping to fit...");
        fov = Math::Clamp(fov, 1.0f, 180.0f);
    }

    // Dig down to actually set it.
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        camera->SetCameraFovDegrees(fov);
    }
    return 0;
}
RegFunc1(SetCameraFOV, void, float, IMMEDIATE, REL_FUNC);

float GetCameraClipBack()
{
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        return camera->GetFarClipPlaneDistance();
    }
    return 0.0f;
}
RegFunc0(GetCameraClipBack, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraClipBack(float distance)
{
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        camera->SetFarClipPlaneDistance(distance);
    }
    return 0;
}
RegFunc1(SetCameraClipBack, void, float, IMMEDIATE, REL_FUNC);

float GetCameraClipFront()
{
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        return camera->GetNearClipPlaneDistance();
    }
    return 0.0f;
}
RegFunc0(GetCameraClipFront, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraClipFront(float distance)
{
    Camera* camera = GetCamera();
    if(camera != nullptr)
    {
        camera->SetNearClipPlaneDistance(distance);
    }
    return 0;
}
RegFunc1(SetCameraClipFront, void, float, IMMEDIATE, REL_FUNC);

shpvoid EnableCinematics()
{
    GameCamera::SetCinematicsEnabled(true);
    return 0;
}
RegFunc0(EnableCinematics, void, IMMEDIATE, REL_FUNC);

shpvoid DisableCinematics()
{
    GameCamera::SetCinematicsEnabled(false);
    return 0;
}
RegFunc0(DisableCinematics, void, IMMEDIATE, REL_FUNC);

shpvoid EnableCameraBoundaries()
{
    gSceneManager.GetScene()->GetCamera()->SetBoundsEnabled(true);
    return 0;
}
RegFunc0(EnableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);

shpvoid DisableCameraBoundaries()
{
    // Note that this only disables camera boundaries until the next scene load.
    // This reflects the behavior in the OG game.
    gSceneManager.GetScene()->GetCamera()->SetBoundsEnabled(false);
    return 0;
}
RegFunc0(DisableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);


shpvoid CameraBoundaryBlockModel(const std::string& modelName)
{
    GKObject* object = gSceneManager.GetScene()->GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        MeshRenderer* meshRenderer = object->GetMeshRenderer();
        if(meshRenderer != nullptr)
        {
            gSceneManager.GetScene()->GetCamera()->AddBounds(meshRenderer->GetModel());
        }
    }
    return 0;
}
RegFunc1(CameraBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid CameraBoundaryUnblockModel(const std::string& modelName)
{
    GKObject* object = gSceneManager.GetScene()->GetSceneObjectByModelName(modelName);
    if(object != nullptr)
    {
        MeshRenderer* meshRenderer = object->GetMeshRenderer();
        if(meshRenderer != nullptr)
        {
            gSceneManager.GetScene()->GetCamera()->RemoveBounds(meshRenderer->GetModel());
        }
    }
    return 0;
}
RegFunc1(CameraBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid GlideToCameraAngle(const std::string& cameraName)
{
    gSceneManager.GetScene()->GlideToCameraPosition(cameraName, AddWait());
    return 0;
}
RegFunc1(GlideToCameraAngle, void, string, WAITABLE, REL_FUNC);

shpvoid CutToCameraAngle(const std::string& cameraName)
{
    // This version of the function only cuts the camera if cinematics are enabled, OR forced cinematics is temporarily enabled.
    if(GameCamera::AreCinematicsEnabled() || gSceneManager.GetScene()->GetCamera()->IsForcedCinematicMode())
    {
        gSceneManager.GetScene()->SetCameraPosition(cameraName);
    }
    return 0;
}
RegFunc1(CutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid CutToCameraAngleX(float horizAngle, float vertAngle, float x, float y, float z) // DEV
{
    return 0;
}
RegFunc5(CutToCameraAngleX, void, float, float, float, float, float, IMMEDIATE, DEV_FUNC);
*/

shpvoid ForceCutToCameraAngle(const std::string& cameraName)
{
    gSceneManager.GetScene()->SetCameraPosition(cameraName);
    return 0;
}
RegFunc1(ForceCutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetForcedCameraCuts(int flag)
{
    gSceneManager.GetScene()->GetCamera()->SetForcedCinematicMode(flag != 0 ? true : false);
    return 0;
}
RegFunc1(SetForcedCameraCuts, void, int, IMMEDIATE, REL_FUNC);

shpvoid ClearForcedCameraCuts()
{
    gSceneManager.GetScene()->GetCamera()->SetForcedCinematicMode(false);
    return 0;
}
RegFunc0(ClearForcedCameraCuts, void, IMMEDIATE, REL_FUNC);

shpvoid SetCameraAngleType(const std::string& cameraName, const std::string& type)
{
    printf("SetCameraAngleType(%s, %s)\n", cameraName.c_str(), type.c_str());
    return 0;
}
RegFunc2(SetCameraAngleType, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid DefaultInspect(const std::string& noun)
{
    gSceneManager.GetScene()->InspectObject(noun, AddWait());
    return 0;
}
RegFunc1(DefaultInspect, void, string, WAITABLE, REL_FUNC);

shpvoid InspectObject()
{
    gSceneManager.GetScene()->InspectActiveObject(AddWait());
    return 0;
}
RegFunc0(InspectObject, void, WAITABLE, REL_FUNC);

shpvoid Uninspect()
{
    gSceneManager.GetScene()->UninspectObject(AddWait());
    return 0;
}
RegFunc0(Uninspect, void, WAITABLE, REL_FUNC);

shpvoid InspectModelUsingAngle(const std::string& modelName, const std::string& cameraName)
{
    printf("InspectModelUsingAngle(%s, %s)\n", modelName.c_str(), cameraName.c_str());
    return 0;
}
RegFunc2(InspectModelUsingAngle, void, string, string, IMMEDIATE, REL_FUNC);