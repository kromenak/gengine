//
// SheepAPI.h
//
// Clark Kromenaker
//
// Provides all Sheep API system functions.
// All functions that can be called from sheep are in this header.
//
#pragma once
#include <initializer_list>
#include <map>
#include <string>

#include "AtomicTypes.h"
#include "SheepScript.h"
#include "Value.h"

// Functions for calling functions of various argument lengths.
Value CallSysFunc(const std::string& name, const Value& x1);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2);

extern std::vector<SysImport> sysFuncs;

void AddSysImport(const std::string& name, char retType, std::initializer_list<char> argTypes);

#define void_TYPE 0
#define int_TYPE 1
#define float_TYPE 2
#define string_TYPE 3

// Macros that register functions of various argument lengths with the system.
// Creates a function with same name as the actual function, but which uses generic "Value" args and return type.
// The generic function just calls the real function with correct argument types.

// Also registers an entry in the function map with a pointer to the "generic function".
// Flow is: Look Up in Map -> Calls Generic Function -> Calls Actual Function
#define RegFunc0(name, ret)                         \
    Value name(const Value& x1) {                   \
        return name();                              \
    }                                               \
    struct name##_ {                                \
        name##_() {                                 \
            map0[#name]=&name;                      \
            AddSysImport(#name, ret##_TYPE, { }); \
        }                                           \
    } name##_instance

#define RegFunc1(name, ret, t1)                     \
    Value name(const Value& x1) {                   \
        return name(x1.to<t1>());                   \
    }                                               \
    struct name##_ {                                \
        name##_() {                                 \
            map1[#name]=&name;                      \
            AddSysImport(#name, ret##_TYPE, { t1##_TYPE }); \
        }                                           \
    } name##_instance

#define RegFunc2(name, ret, t1, t2)                              \
    Value name(const Value& x1, const Value& x2) {          \
        return name(x1.to<t1>(), x2.to<t2>());              \
    }                                                       \
    struct name##_ {                                        \
        name##_() {                                         \
            map2[#name]=&name;                              \
            AddSysImport(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE }); \
        }                                                   \
    } name##_instance

#define shpvoid uint8_t

// ACTORS
shpvoid Blink(std::string actorName);
shpvoid BlinkX(std::string actorName, std::string blinkAnim);

shpvoid ClearMood(std::string actorName);

shpvoid EnableEyeJitter(std::string actorName);
shpvoid DisableEyeJitter(std::string actorName);
shpvoid EyeJitter(std::string actorName);

shpvoid DumpActorPosition(std::string actorName); // DEV

shpvoid Expression(std::string actorName, std::string expression);

int GetEgoCurrentLocationCount();
int GetEgoLocationCount(std::string locationName);

std::string GetEgoName();

std::string GetIndexedPosition(int index); // DEV
int GetPositionCount(); // DEV

shpvoid Glance(std::string actorName, int percentX, int percentY, int durationMs); // WAIT
shpvoid GlanceX(std::string actorName, int leftPercentX, int leftPercentY,
                int rightPercentX, int rightPercentY, int durationMs); // WAIT

shpvoid InitEgoPosition(std::string positionName);

int IsActorAtLocation(std::string actorName, std::string locationName);
int IsActorNear(std::string actorName, std::string positionName, float distance);
int IsWalkingActorNear(std::string actorName, std::string positionName, float distance);

int IsActorOffstage(std::string actorName);

int IsCurrentEgo(std::string actorName);

shpvoid LookitActor(std::string actorName, std::string otherActorName,
                    std::string componentsSpec, float durationSec);
shpvoid LookitActorQuick(std::string actorName, std::string otherActorName,
                         std::string componentsSpec, float durationSec);

shpvoid LookitModel(std::string actorName, std::string modelName,
                    std::string componentsSpec, float durationSec);
shpvoid LookitModelQuick(std::string actorName, std::string modelName,
                         std::string componentsSpec, float durationSec);
shpvoid LookitModelX(std::string actorName, std::string modelName, int mesh,
                     std::string boxModifier, float offsetX, float offsetY, float offsetZ,
                     std::string componentsSpec, float durationSec);
shpvoid LookitModelQuickX(std::string actorName, std::string modelName, int mesh,
                          std::string boxModifier, float offsetX, float offsetY, float offsetZ,
                          std::string componentsSpec, float durationSec);

shpvoid LookitNoun(std::string actorName, std::string nounName,
                   std::string componentsSpec, float durationSec);
shpvoid LookitNounQuick(std::string actorName, std::string nounName,
                        std::string componentsSpec, float durationSec);

shpvoid LookitPoint(std::string actorName, float x, float y, float z,
                    std::string componentsSpec, float durationSec);

shpvoid LookitCameraAngle(std::string actorName, std::string cameraAngleName,
                          std::string componentsSpec, float durationSec); // DEV
shpvoid LookitMouse(std::string actorName, std::string componentsSpec, float durationSec); // DEV
shpvoid LookitPlayer(std::string actorName, std::string componentsSpec, float durationSec); // DEV

shpvoid LookitCancel(std::string actorName);

shpvoid LookitLock(std::string actorName, std::string componentsSpec, float durationSec);
shpvoid LookitUnlock(std::string actorName, std::string componentsSpec);

shpvoid SetActorLocation(std::string actorName, std::string locationName);
shpvoid SetActorOffstage(std::string actorName);

shpvoid SetActorPosition(std::string actorName, std::string positionName);

shpvoid SetEyeOffsets(std::string actorName, float leftX, float leftY,
                      float rightX, float rightY); // DEV

shpvoid SetEgo(std::string actorName); // DEV
shpvoid SetEgoLocationCount(std::string locationName, int count); // DEV

shpvoid SetIdleGAS(std::string actorName, std::string gasName); // WAIT
shpvoid SetListenGAS(std::string actorName, std::string gasName); // WAIT
shpvoid SetTalkGAS(std::string actorName, std::string gasName); // WAIT

shpvoid SetMood(std::string actorName, std::string moodName);

shpvoid SetNextEgo(); // DEV
shpvoid SetPrevEgo(); // DEV

shpvoid SetWalkAnim(std::string actorName, std::string start, std::string cont,
                    std::string startTurnLeft, std::string startTurnRight);

shpvoid StartIdleFidget(std::string actorName); // WAIT
shpvoid StartListenFidget(std::string actorName); // WAIT
shpvoid StartTalkFidget(std::string actorName); // WAIT
shpvoid StopFidget(std::string actorName); // WAIT

shpvoid TurnHead(std::string actorName, int percentX, int percentY, int durationMs); // WAIT
shpvoid TurnToModel(std::string actorName, std::string modelName); // WAIT

shpvoid WalkerBoundaryBlockModel(std::string modelName);
shpvoid WalkerBoundaryBlockRegion(int regionIndex);
shpvoid WalkerBoundaryUnblockModel(std::string modelName);
shpvoid WalkerBoundaryUnblockRegion(int regionIndex);

shpvoid WalkNear(std::string actorName, std::string positionName); // WAIT
shpvoid WalkNearModel(std::string actorName, std::string modelName); // WAIT
shpvoid WalkTo(std::string actorName, std::string positionName); // WAIT
shpvoid WalkToAnimation(std::string actorName, std::string animationName); // WAIT
shpvoid WalkToSeeModel(std::string actorName, std::string modelName); // WAIT
shpvoid WalkToXZ(std::string actorName, float xPos, float zPos); // DEV, WAIT

int WasEgoEverInLocation(std::string locationName);

// ANIMATION AND DIALOGUE
shpvoid AddCaptionDefault(std::string captionText); // DEV
shpvoid AddCaptionEgo(std::string captionText); // DEV
shpvoid AddCaptionVoiceOver(std::string captionText); // DEV
shpvoid ClearCaptionText(); // DEV

shpvoid AnimEvent(std::string eventType, std::string eventData); // DEV

shpvoid StartDialogue(std::string dialogueName, int numLines); // WAIT
shpvoid StartDialogueNoFidgets(std::string dialogueName, int numLines); // WAIT
shpvoid ContinueDialogue(int numLines); // WAIT
shpvoid ContinueDialogueNoFidgets(int numLines); // WAIT

shpvoid EnableInterpolation(); // DEV
shpvoid DisableInterpolation(); // DEV

shpvoid DumpAnimator(); // DEV

shpvoid SetConversation(std::string conversationName); // WAIT
shpvoid EndConversation(); // WAIT

shpvoid StartAnimation(std::string animName); // WAIT
shpvoid LoopAnimation(std::string animName);
shpvoid StopAnimation(std::string animName);
shpvoid StopAllAnimations(); // DEV

shpvoid StartMorphAnimation(std::string animName, int animStartFrame, int morphFrames); // WAIT
shpvoid StopMorphAnimation(std::string animName);
shpvoid StopAllMorphAnimations(); // DEV

shpvoid StartMom(std::string momAnimName); // WAIT
shpvoid StartMoveAnimation(std::string animName); // WAIT

shpvoid StartVoiceOver(std::string dialogueName, int numLines); // WAIT
shpvoid StartYak(std::string yakAnimName); // DEV, WAIT

// APPLICATION
shpvoid AddPath(std::string pathName); // DEV
shpvoid FullScanPaths(); // DEV
shpvoid RescanPaths(); // DEV

shpvoid DumpBuildInfo(); // DEV
shpvoid DumpLayerStack(); // DEV

shpvoid Edit(std::string filename); // DEV
shpvoid Open(std::string filename); // DEV

shpvoid ForceQuitGame(); // DEV

shpvoid FullReset(); // DEV

shpvoid QuitApp(); // DEV

shpvoid RefreshScreen(); // DEV

shpvoid Screenshot();
shpvoid ScreenshotX(std::string filename); // DEV

shpvoid ShowBinocs();
shpvoid ShowDrivingInterface();
shpvoid ShowFingerprintInterface(std::string nounName);
shpvoid ShowSidney();

shpvoid StartGame(); // DEV

// CAMERA
shpvoid CameraBoundaryBlockModel(std::string modelName);
shpvoid CameraBoundaryUnblockModel(std::string modelName);

shpvoid CutToCameraAngle(std::string cameraName);
shpvoid CutToCameraAngleX(float horizAngle, float vertAngle,
                          float x, float y, float z); // DEV

shpvoid DefaultInspect(std::string noun); // WAIT

shpvoid DisableCameraBoundaries(); // DEV
shpvoid DumpCamera(); // DEV
shpvoid DumpCameraAngles(); // DEV
shpvoid EnableCameraBoundaries(); // DEV

shpvoid ForceCutToCameraAngle(std::string cameraName);

int GetCameraAngleCount(); // DEV
std::string GetIndexedCameraAngle(int index); // DEV

shpvoid SetCameraGlide(int glide);
shpvoid GlideToCameraAngle(std::string cameraName); // WAIT
shpvoid InspectObject(); // WAIT
shpvoid Uninspect(); // WAIT

float GetCameraFOV();
shpvoid SetCameraFOV(float fov);

// CONSTRUCTION MODE
std::string CreateCameraAngleGizmo(); // DEV
std::string CreateCameraAngleGizmoX(float horizAngle, float vertAngle,
                                    float x, float y, float z); // DEV

std::string CreatePositionGizmo(); // DEV
std::string CreatePositionGizmoX(float heading, float x, float y, float z); // DEV

shpvoid HideAmbientMapGizmo(); // DEV
shpvoid HideCameraAngleGizmo(std::string cameraName); // DEV
shpvoid HideConstruction(); // DEV
shpvoid HidePositionGizmo(std::string positionName); // DEV
shpvoid HideWalkerBoundaryGizmo(); // DEV

shpvoid SetSceneViewport(int xPercent, int yPercent, int widthPercent, int heightPercent); // DEV

shpvoid ShowAmbientMapGizmo(); // DEV
shpvoid ShowCameraAngleGizmo(std::string cameraName); // DEV
shpvoid ShowConstruction(); // DEV
shpvoid ShowPositionGizmo(); // DEV
shpvoid ShowWalkerBoundaryGizmo(); // DEV

shpvoid TextInpsectCameraGizmo(std::string cameraName); // DEV
shpvoid TextInspectCameraGizmoX(std::string cameraName, int xPercent,
                                int yPercent, int fontSize); // DEV
shpvoid TextInpsectPositionGizmo(std::string positionName); // DEV
shpvoid TextInspectPositionGizmoX(std::string positionName, int xPercent,
                                int yPercent, int fontSize); // DEV

shpvoid ViewportInspectCameraGizmo(std::string cameraName); // DEV
shpvoid ViewportInspectCameraGizmoX(std::string cameraName, int xPercent,
                                    int yPercent, int widthPercent, int heightPercent); // DEV

// DEBUGGING (ALL DEV)
shpvoid AddTemplate(std::string templateText, std::string expandedText, int removeTemplate);
shpvoid Alias(std::string alias, std::string sheepCommand);
shpvoid BindDebugKey(std::string keyName, std::string sheepCommand);
shpvoid ClearConsole();
shpvoid ClearConsoleBuffer();
shpvoid ClearDebugFlag(std::string flagName);
shpvoid CloseConsole();
shpvoid DumpDebugFlags();
shpvoid DumpFile(std::string filename);
shpvoid DumpLockedObjects();
shpvoid DumpMemoryUsage();
shpvoid DumpPathFileMap();
shpvoid DumpUsedPaths();
shpvoid DumpUsedFiles();
int GetDebugFlag(std::string flagName);
float GetTimeMultiplier();
shpvoid InsertConsole(std::string command);
shpvoid OpenConsole();
shpvoid RemoveTemplate(std::string templateText);
shpvoid ReportMemoryUsage();
shpvoid ReportSurfaceMemoryUsage();
shpvoid SetConsole(std::string command);
shpvoid SetDebugFlag(std::string flagName);
shpvoid SetTimeMultiplier(float multiplier);
shpvoid ToggleConsole();
shpvoid ToggleDebugFlag(std::string flagName);
shpvoid Unalias(std::string alias);
shpvoid UnbindDebugKey(std::string keyName);

// ENGINE
shpvoid Call(std::string functionName); // WAIT
shpvoid CallDefaultSheep(std::string sheepFileName); // WAIT
shpvoid CallGlobal(std::string functionName); // WAIT
shpvoid CallGlobalSheep(std::string sheepFileName, std::string functionName); // WAIT

shpvoid CallIndexedSheep(std::string sheepFileName, int sheepIndex, std::string functionName); // DEV, WAIT

shpvoid CallSheep(std::string sheepFileName, std::string functionName); // WAIT

shpvoid DisableCinematics(); // DEV
shpvoid DisableIncrementalRendering(); // DEV
shpvoid DisableCurrentSheepCaching(); // DEV
shpvoid DisableSheepCaching(); // DEV

shpvoid DumpActiveSheepObjects(); // DEV
shpvoid DumpActiveSheepThreads(); // DEV
shpvoid DumpCommands(); // DEV
shpvoid DumpRawSheep(std::string sheepName); // DEV
shpvoid DumpSheepEngine(); // DEV

shpvoid EnableCinematics(); // DEV
shpvoid EnableIncrementalRendering(); // DEV
shpvoid EnableSheepCaching(); // DEV

shpvoid ExecCommand(std::string sheepCommand); // DEV
shpvoid FindCommand(std::string commandGuess); // DEV

std::string GetCurrentSheepFunction();
std::string GetCurrentSheepName();

float GetGamma(); // DEV
shpvoid SetGamma(float gamma); // DEV

shpvoid HelpCommand(std::string commandName); // DEV

shpvoid NukeAllSheep(); // DEV
shpvoid NukeSheep(std::string sheepName); // DEV

shpvoid SaveSprite(std::string spriteName, std::string fileName); // DEV
shpvoid SaveTexture(std::string textureName, std::string fileName); // DEV
shpvoid SaveTextureX(std::string textureName, int surfaceIndex, std::string fileName); // DEV

shpvoid SetGlobalSheep();

shpvoid SetRenderFlat(); // DEV
shpvoid SetRenderFull(); // DEV
shpvoid SetRenderShaded(); // DEV
shpvoid SetRenderWireframe(); // DEV

shpvoid SetShadowTypeBlobby(); // DEV
shpvoid SetShadowTypeModel(); // DEV
shpvoid SetShadowTypeNone(); // DEV

shpvoid SetSurfaceHigh(); // DEV
shpvoid SetSurfaceLow(); // DEV
shpvoid SetSurfaceNormal(); // DEV

shpvoid SetTimerMs(int milliseconds); // WAIT
shpvoid SetTimerSeconds(float seconds); // WAIT

shpvoid SetTopSheep();

shpvoid ThrowException(); // DEV

shpvoid UnloadAll(); // DEV
shpvoid UnloadAllAnimations(); // DEV
shpvoid UnloadAllModels(); // DEV
shpvoid UnloadAllMovies(); // DEV
shpvoid UnloadAllScenes(); // DEV
shpvoid UnloadAllSounds(); // DEV
shpvoid UnloadAllSprites(); // DEV
shpvoid UnloadAllTextures(); // DEV

shpvoid UnloadAnimation(std::string animName); // DEV
shpvoid UnloadModel(std::string modelName); // DEV
shpvoid UnloadMovie(std::string movieName); // DEV
shpvoid UnloadScene(std::string sceneName); // DEV
shpvoid UnloadSound(std::string soundName); // DEV
shpvoid UnloadSprite(std::string spriteName); // DEV
shpvoid UnloadTexture(std::string textureName); // DEV

// GAME LOGIC
shpvoid AddCaseLogic(std::string caseVal, std::string logic); // DEV
shpvoid CheckCase(std::string noun, std::string verb, std::string caseVal); // DEV
shpvoid CommitCaseLogic(); // DEV
shpvoid DumpCaseCode(); // DEV
shpvoid ResetCaseLogic(); // DEV

shpvoid ClearFlag(std::string flagName);
shpvoid DumpFlags(); // DEV
int GetFlag(std::string flagName);
int GetFlagInt(int flagEnum);
shpvoid SetFlag(std::string flagName);

shpvoid DumpNouns(); // DEV

int GetChatCount(std::string noun);
int GetChatCountInt(int nounEnum);
shpvoid SetChatCount(std::string noun, int count); // DEV

int GetGameVariableInt(std::string varName);
shpvoid IncGameVariableInt(std::string varName);
shpvoid SetGameVariableInt(std::string varName, int value);

int GetNounVerbCount(std::string noun, std::string verb);
int GetNounVerbCountInt(int nounEnum, int verbEnum);
shpvoid IncNounVerbCount(std::string noun, std::string verb);
shpvoid SetNounVerbCount(std::string noun, std::string verb, int count);
shpvoid TriggerNounVerb(std::string noun, std::string verb);

// ChangeScore???
int GetScore();
shpvoid IncreaseScore(int value);
shpvoid SetScore(int score); // DEV

int GetTopicCount(std::string noun, std::string verb);
int GetTopicCountInt(int nounEnum, int verbEnum);
int HasTopicsLeft(std::string noun);
shpvoid SetTopicCount(std::string noun, std::string verb, int count); // DEV

int IsCurrentLocation(std::string location);
int IsCurrentTime(std::string timeCode);
int WasLastLocation(std::string lastLocation);
int WasLastTime(std::string lastTimeCode);

shpvoid ResetGameData(); // DEV

shpvoid SetGameTimer(std::string noun, std::string verb, int milliseconds);

// GENERAL
shpvoid DrawFilledRect(int left, int top, int right, int bottom,
                       int red, int green, int blue); // DEV

shpvoid FollowOnDrivingMap(int follwState); // WAIT

float GetRandomFloat(float lower, float upper);
int GetRandomInt(int lower, int upper);

shpvoid PlayFullScreenMovie(std::string movieName); // WAIT
shpvoid PlayMovie(std::string movieName); // WAIT

shpvoid SetPamphletPage(int page);

// INSETS
shpvoid DumpInsetNames(); // DEV
shpvoid HideInset();
shpvoid HidePlate(std::string plateName);
shpvoid ShowInset(std::string insetName);
shpvoid ShowPlate(std::string plateName);

// INVENTORY
shpvoid CombineInvItems(std::string firstItemName, std::string secondItemName,
                        std::string combinedItemName);

int DoesEgoHaveInvItem(std::string itemName);
int DoesGabeHaveInvItem(std::string itemName);
int DoesGraceHaveInvItem(std::string itemName);
shpvoid EgoTakeInvItem(std::string itemName);

shpvoid DumpEgoActiveInvItem(); // DEV
shpvoid SetEgoActiveInvItem(std::string itemName);

shpvoid HideInventory();
shpvoid ShowInventory();

shpvoid InventoryInspect(std::string itemName);
shpvoid InventoryUninspect();

shpvoid SetInvItemStatus(std::string itemName, std::string status);

// MODELS
shpvoid ClearModelShadowTexture(std::string modelName);
shpvoid SetModelShadowTexture(std::string modelName, std::string textureName);

shpvoid ClearGasProp(std::string modelName);
shpvoid SetPropGas(std::string modelName, std::string gasName);

int DoesModelExist(std::string modelName);
int DoesSceneModelExist(std::string modelName);

shpvoid DumpModel(std::string modelName); // DEV
shpvoid DumpModelNames(); // DEV
shpvoid DumpSceneModelNames(); // DEV

shpvoid HideModel(std::string modelName);
shpvoid HideModelGroup(std::string groupName);
shpvoid HideSceneModel(std::string modelName);

int IsModelVisible(std::string modelName);
int IsSceneModelVisible(std::string modelName);

shpvoid ShowModel(std::string modelName);
shpvoid ShowModelGroup(std::string groupName);
shpvoid ShowSceneModel(std::string modelName);

shpvoid StartPropFidget(std::string modelName);
shpvoid StopPropFidget(std::string modelName);

// REPORTS (ALL DEV)
shpvoid AddStreamContent(std::string streamName, std::string content);
shpvoid ClearStreamContent(std::string streamName);
shpvoid RemoveStreamContent(std::string streamName, std::string content);

shpvoid AddStreamOutput(std::string streamName, std::string output);
shpvoid ClearStreamOutput(std::string streamName);
shpvoid RemoveStreamOutput(std::string streamName, std::string output);

shpvoid DisableStream(std::string stream);
shpvoid EnableStream(std::string stream);

shpvoid HideReportGraph(std::string graphType);
shpvoid ShowReportGraph(std::string graphType);

shpvoid SetStreamAction(std::string streamName, std::string action);
shpvoid SetStreamFilename(std::string streamName, std::string filename);
shpvoid SetStreamFileTruncate(std::string streamName, int truncate);

// SCENE
shpvoid CallSceneFunction(std::string parameter); // WAIT

shpvoid DumpLocations(); // DEV
shpvoid DumpPosition(std::string positionName); // DEV
shpvoid DumpPositions(); // DEV
shpvoid DumpTimes(); // DEV

shpvoid ReEnter(); // DEV, WAIT

shpvoid SetLocation(std::string location); // WAIT
shpvoid SetLocationTime(std::string location, std::string time); // WAIT
shpvoid SetScene(std::string sceneName); // WAIT
shpvoid SetTime(std::string time); // WAIT

shpvoid Warp(std::string locationAndTime); // WAIT

// SOUND
shpvoid DisableSound(std::string soundType); // DEV
shpvoid EnableSound(std::string soundType); // DEV
shpvoid GetVolume(std::string soundType); // DEV
shpvoid SetVolume(std::string soundType); // DEV

shpvoid PlaySound(std::string soundName); // WAIT
shpvoid PlaySoundTrack(std::string soundtrackName); // WAIT

shpvoid StopAllSounds();
shpvoid StopAllSoundTracks();
shpvoid StopSound(std::string soundName);
shpvoid StopSoundTrack(std::string soundtrackName); // WAIT

// TRACING (ALL DEV)
shpvoid PrintFloat(float value);
shpvoid PrintFloatX(std::string category, float value);

shpvoid PrintInt(int value);
shpvoid PrintIntX(std::string category, int value);
shpvoid PrintIntHex(int value);
shpvoid PrintIntHexX(std::string category, int value);

shpvoid PrintString(std::string message);
shpvoid PrintStringX(std::string category, std::string message);













