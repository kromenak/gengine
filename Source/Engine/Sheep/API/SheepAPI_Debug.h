//
// Clark Kromenaker
//
// Sheep SysFuncs related to debugging and development tools.
//
#pragma once
#include "SheepSysFunc.h"

int GetDebugFlag(const std::string& flagName);
shpvoid SetDebugFlag(const std::string& flagName);
shpvoid ClearDebugFlag(const std::string& flagName);
shpvoid ToggleDebugFlag(const std::string& flagName);
shpvoid DumpDebugFlags();

shpvoid BindDebugKey(const std::string& keyName, const std::string& sheepCommand);
shpvoid UnbindDebugKey(const std::string& keyName);

shpvoid DumpFile(const std::string& filename);
shpvoid DumpPathFileMap();
shpvoid DumpUsedPaths();
shpvoid DumpUsedFiles();
shpvoid DumpLayerStack(); // DEV
shpvoid DumpBuildInfo(); // DEV
shpvoid DumpActionManager(); // DEV
shpvoid DumpUIStates(); // DEV

shpvoid ReportMemoryUsage();
shpvoid ReportSurfaceMemoryUsage();

shpvoid DebugBreak(); // DEV
shpvoid ThrowException(); // DEV
shpvoid Death();
shpvoid Death2();

// CONSOLE
shpvoid OpenConsole();
shpvoid CloseConsole();
shpvoid ToggleConsole();

shpvoid ClearConsoleBuffer();

shpvoid SetConsole(const std::string& command);
shpvoid InsertConsole(const std::string& command);
shpvoid ClearConsole();

shpvoid AddTemplate(const std::string& templateText, const std::string& expandedText, int removeTemplate);
shpvoid RemoveTemplate(const std::string& templateText);

shpvoid Alias(const std::string& alias, const std::string& sheepCommand);
shpvoid Unalias(const std::string& alias);

// CONSTRUCTION
shpvoid ShowConstruction(); // DEV
shpvoid HideConstruction(); // DEV

std::string CreateCameraAngleGizmo(); // DEV
std::string CreateCameraAngleGizmoX(float horizAngle, float vertAngle,
                                    float x, float y, float z); // DEV
shpvoid ShowCameraAngleGizmo(const std::string& cameraName); // DEV
shpvoid HideCameraAngleGizmo(const std::string& cameraName); // DEV

std::string CreatePositionGizmo(); // DEV
std::string CreatePositionGizmoX(float heading, float x, float y, float z); // DEV
shpvoid ShowPositionGizmo(const std::string& positionName); // DEV
shpvoid HidePositionGizmo(const std::string& positionName); // DEV

shpvoid ShowWalkerBoundaryGizmo(); // DEV
shpvoid HideWalkerBoundaryGizmo(); // DEV

shpvoid SetSceneViewport(int xPercent, int yPercent, int widthPercent, int heightPercent); // DEV

shpvoid TextInspectCameraGizmo(const std::string& cameraName); // DEV
shpvoid TextInspectCameraGizmoX(const std::string& cameraName, int xPercent,
                                int yPercent, int fontSize); // DEV

shpvoid TextInspectPositionGizmo(const std::string& positionName); // DEV
shpvoid TextInspectPositionGizmoX(const std::string& positionName, int xPercent,
                                  int yPercent, int fontSize); // DEV

shpvoid ViewportInspectCameraGizmo(const std::string& cameraName); // DEV
shpvoid ViewportInspectCameraGizmoX(const std::string& cameraName, int xPercent,
                                    int yPercent, int widthPercent, int heightPercent); // DEV

// MISC
