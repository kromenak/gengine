#include "SheepAPI_Camera.h"

#include "Camera.h"
#include "GameCamera.h"
#include "GEngine.h"
#include "Scene.h"

using namespace std;

float GetCameraFOV()
{
    // Dig down to grab the value.
    GameCamera* gameCamera = GEngine::Instance()->GetScene()->GetCamera();
    if(gameCamera != nullptr)
    {
        Camera* camera = gameCamera->GetCamera();
        if(camera != nullptr)
        {
            return camera->GetCameraFovDegrees();
        }
    }
    return 0.0f;
}
RegFunc0(GetCameraFOV, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraFOV(float fov)
{
    // Clamp argument in valid range.
    if(fov < 1.0f || fov > 180.0f)
    {
        Services::GetReports()->Log("Warning", "Warning: camera FOV must be between 1 and 180. Clamping to fit...");
        fov = Math::Clamp(fov, 1.0f, 180.0f);
    }

    // Dig down to actually set it.
    GameCamera* gameCamera = GEngine::Instance()->GetScene()->GetCamera();
    if(gameCamera != nullptr)
    {
        Camera* camera = gameCamera->GetCamera();
        if(camera != nullptr)
        {
            camera->SetCameraFovDegrees(fov);
        }
    }
    return 0;
}
RegFunc1(SetCameraFOV, void, float, IMMEDIATE, REL_FUNC);

shpvoid EnableCameraBoundaries()
{
    GEngine::Instance()->GetScene()->GetCamera()->SetBoundsEnabled(true);
    return 0;
}
RegFunc0(EnableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);

shpvoid DisableCameraBoundaries()
{
    // Note that this only disables camera boundaries until the next scene load.
    // This reflects the behavior in the OG game.
    GEngine::Instance()->GetScene()->GetCamera()->SetBoundsEnabled(false);
    return 0;
}
RegFunc0(DisableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);

/*
shpvoid CameraBoundaryBlockModel(std::string modelName)
{
    std::cout << "CameraBoundaryBlockModel" << std::endl;
    return 0;
}
RegFunc1(CameraBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid CameraBoundaryUnblockModel(std::string modelName)
{
    std::cout << "CameraBoundaryUnblockModel" << std::endl;
    return 0;
}
RegFunc1(CameraBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);
*/

shpvoid GlideToCameraAngle(const std::string& cameraName)
{
    GEngine::Instance()->GetScene()->GlideToCameraPosition(cameraName, AddWait());
    return 0;
}
RegFunc1(GlideToCameraAngle, void, string, WAITABLE, REL_FUNC);

shpvoid CutToCameraAngle(const std::string& cameraName)
{
    GEngine::Instance()->GetScene()->SetCameraPosition(cameraName);
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
    GEngine::Instance()->GetScene()->SetCameraPosition(cameraName);
    return 0;
}
RegFunc1(ForceCutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

shpvoid DefaultInspect(const std::string& noun)
{
    GEngine::Instance()->GetScene()->InspectObject(noun, AddWait());
    return 0;
}
RegFunc1(DefaultInspect, void, string, WAITABLE, REL_FUNC);

shpvoid InspectObject()
{
    GEngine::Instance()->GetScene()->InspectActiveObject(AddWait());
    return 0;
}
RegFunc0(InspectObject, void, WAITABLE, REL_FUNC);

shpvoid Uninspect()
{
    GEngine::Instance()->GetScene()->UninspectObject(AddWait());
    return 0;
}
RegFunc0(Uninspect, void, WAITABLE, REL_FUNC);
