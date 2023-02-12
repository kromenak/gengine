//
// Clark Kromenaker
//
// Sheep SysFuncs related to camera settings and behavior.
//
#pragma once
#include "SheepSysFunc.h"

// CAMERA PROPERTIES
shpvoid SetCameraGlide(int glide);

float GetCameraFOV();
shpvoid SetCameraFOV(float fov);

shpvoid EnableCinematics(); // DEV
shpvoid DisableCinematics(); // DEV

shpvoid DumpCamera(); // DEV

// CAMERA BOUNDARIES
shpvoid EnableCameraBoundaries(); // DEV
shpvoid DisableCameraBoundaries(); // DEV

shpvoid CameraBoundaryBlockModel(const std::string& modelName);
shpvoid CameraBoundaryUnblockModel(const std::string& modelName);

// CAMERA ANGLES
shpvoid GlideToCameraAngle(const std::string& cameraName); // WAIT

shpvoid CutToCameraAngle(const std::string& cameraName);
shpvoid CutToCameraAngleX(float horizAngle, float vertAngle, float x, float y, float z); // DEV
shpvoid ForceCutToCameraAngle(const std::string& cameraName);

int GetCameraAngleCount(); // DEV
std::string GetIndexedCameraAngle(int index); // DEV

shpvoid DumpCameraAngles(); // DEV

// CAMERA INSPECT
shpvoid DefaultInspect(const std::string& noun); // WAIT
shpvoid InspectObject(); // WAIT
shpvoid Uninspect(); // WAIT
