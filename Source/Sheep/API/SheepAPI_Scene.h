//
// Clark Kromenaker
//
// Sheep SysFuncs related to querying or manipulating the scene or
// coordinating multiple scene objects.
//
#pragma once
#include "SheepSysFunc.h"

shpvoid ReEnter(); // DEV, WAIT
shpvoid CallSceneFunction(const std::string& parameter); // WAIT

// EGO
shpvoid SetEgo(const std::string& actorName);
int IsCurrentEgo(const std::string& actorName);
std::string GetEgoName();

shpvoid InitEgoPosition(const std::string& positionName);

shpvoid SetNextEgo(); // DEV
shpvoid SetPrevEgo(); // DEV

// POSITIONS
shpvoid DumpPosition(const std::string& positionName); // DEV
shpvoid DumpPositions(); // DEV

shpvoid SetActorPosition(const std::string& actorName, const std::string& positionName);
shpvoid DumpActorPosition(const std::string& actorName); // DEV

int IsActorNear(const std::string& actorName, const std::string& positionName, float distance);
int IsWalkingActorNear(const std::string& actorName, const std::string& positionName, float distance);

std::string GetIndexedPosition(int index); // DEV
int GetPositionCount(); // DEV

// ACTORS
int DoesActorExist(const std::string& actorName);

// MODELS
int DoesModelExist(const std::string& modelName);
shpvoid DumpModelNames(); // DEV

int IsModelVisible(const std::string& modelName);
shpvoid ShowModel(const std::string& modelName);
shpvoid HideModel(const std::string& modelName);

// SCENE (BSP) MODELS
int DoesSceneModelExist(const std::string& modelName);
shpvoid DumpSceneModelNames(); // DEV

int IsSceneModelVisible(const std::string& modelName);
shpvoid ShowSceneModel(const std::string& modelName);
shpvoid HideSceneModel(const std::string& modelName);

// HIT TEST (ALSO BSP) MODELS
shpvoid EnableHitTestModel(const std::string& modelName);
shpvoid DisableHitTestModel(const std::string& modelName);

// MODEL GROUPS
shpvoid ShowModelGroup(const std::string& groupName);
shpvoid HideModelGroup(const std::string& groupName);

// WALKING
shpvoid WalkerBoundaryBlockModel(const std::string& modelName);
shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex);
shpvoid WalkerBoundaryUnblockModel(const std::string& modelName);
shpvoid WalkerBoundaryUnblockRegion(int regionIndex, int regionBoundaryIndex);

shpvoid WalkNear(const std::string& actorName, const std::string& positionName); // WAIT
shpvoid WalkNearModel(const std::string& actorName, const std::string& modelName); // WAIT
shpvoid WalkTo(const std::string& actorName, const std::string& positionName); // WAIT
shpvoid WalkToAnimation(const std::string& actorName, const std::string& animationName); // WAIT
shpvoid WalkToSeeModel(const std::string& actorName, const std::string& modelName); // WAIT
shpvoid WalkToXZ(const std::string& actorName, float xPos, float zPos); // DEV, WAIT

// ACTIONS/NVCS
shpvoid AddCaseLogic(const std::string& caseVal, const std::string& logic); // DEV
shpvoid CheckCase(const std::string& noun, const std::string& verb, const std::string& caseVal); // DEV
shpvoid CommitCaseLogic(); // DEV
shpvoid DumpCaseCode(); // DEV
shpvoid ResetCaseLogic(); // DEV

shpvoid DumpNouns(); // DEV

// LIGHTMAPS
shpvoid SetScene(const std::string& sceneName);
shpvoid SetSceneNoPreloadTextures(const std::string& sceneName);

// INSETS
// (NOTE: it doesn't appear that the final game uses the "Inset" system at all, but they are defined in the SIF, so that'd be scene stuff).
shpvoid DumpInsetNames(); // DEV

shpvoid ShowInset(const std::string& insetName);
shpvoid HideInset();

shpvoid ShowPlate(const std::string& plateName);
shpvoid HidePlate(const std::string& plateName);
