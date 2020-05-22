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

// A "full" system function declaration.
// Contains extra data about a function that is helpful, but doesn't uniquely identify the function signature.
// Inherits from SysImport because that DOES uniquely identify the signature!
struct SysFuncDecl : public SysImport
{
	// If true, this function can be "waited" upon.
	// If false, it executes and returns immediately.
	bool waitable = false;
	
	// If true, this function can only work in dev builds.
	bool devOnly = false;
	
	//TODO: For in-game help output, we may need to store argument names AND description text.
	// For example, HelpCommand("AddStreamContent") outputs this:
	/*
	 ----- 'Dump' * 03/16/2019 * 11:39:21 -----
	 ** [DEBUG] AddStreamContent(string streamName, string content) **
	 Adds an additional content type to the stream. Possible values for the 'content' parameter are: 'begin' (report headers), 'content' (report content), 'end' (report footers), 'category' (the category of the report), 'date' (the date the report was made), 'time' (the time the report was made), 'debug' (file/line debug info), 'timeblock' (the current timeblock if there is one), and 'location' (the current game scene location if there is one).
	*/
};

void AddSysFuncDecl(const std::string& name, char retType, std::initializer_list<char> argTypes, bool waitable, bool dev);
SysFuncDecl* GetSysFuncDecl(const std::string& name);
SysFuncDecl* GetSysFuncDecl(const SysImport* sysImport);

// Functions for calling functions of various argument lengths.
Value CallSysFunc(const std::string& name);
Value CallSysFunc(const std::string& name, const Value& x1);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5, const Value& x6);

// These are used in the below macros to convert keywords into integers using ## macro operator.
#define void_TYPE 0
#define int_TYPE 1
#define float_TYPE 2
#define string_TYPE 3

// Macros that register functions of various argument lengths with the system.
// Creates a function with same name as the actual function, but which uses generic "Value" args and return type.
// The generic function just calls the real function with correct argument types.

// Also registers an entry in the function map with a pointer to the "generic function".
// Flow is: Look Up in Map -> Calls Generic Function -> Calls Actual Function
#define RegFunc0(name, ret, waitable, dev)          					\
    Value name(const Value& x1) {                   					\
        return name();                              					\
    }                                               					\
    struct name##_ {                                					\
        name##_() {                                 					\
            map0[#name]=&name;                      					\
            AddSysFuncDecl(#name, ret##_TYPE, { }, waitable, dev); 		\
        }                                           					\
    } name##_instance

#define RegFunc1(name, ret, t1, waitable, dev)                     		\
    Value name(const Value& x1) {                   					\
        return name(x1.to<t1>());                   					\
    }                                               					\
    struct name##_ {                                					\
        name##_() {                                 					\
            map1[#name]=&name;                      					\
            AddSysFuncDecl(#name, ret##_TYPE, { t1##_TYPE }, waitable, dev);			\
        }                                           					\
    } name##_instance

#define RegFunc2(name, ret, t1, t2, waitable, dev)                      \
    Value name(const Value& x1, const Value& x2) {          			\
        return name(x1.to<t1>(), x2.to<t2>());              			\
    }                                                       			\
    struct name##_ {                                        			\
        name##_() {                                         			\
            map2[#name]=&name;                              			\
            AddSysFuncDecl(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE }, waitable, dev); \
        }                                                   			\
    } name##_instance

#define RegFunc3(name, ret, t1, t2, t3, waitable, dev)                      \
	Value name(const Value& x1, const Value& x2, const Value& x3) {          			\
		return name(x1.to<t1>(), x2.to<t2>(), x3.to<t3>());              			\
	}                                                       			\
	struct name##_ {                                        			\
		name##_() {                                         			\
			map3[#name]=&name;                              			\
			AddSysFuncDecl(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE }, waitable, dev); \
		}                                                   			\
	} name##_instance

#define RegFunc4(name, ret, t1, t2, t3, t4, waitable, dev)                      \
	Value name(const Value& x1, const Value& x2, const Value& x3, const Value& x4) {          			\
		return name(x1.to<t1>(), x2.to<t2>(), x3.to<t3>(), x4.to<t4>());              			\
	}                                                       			\
	struct name##_ {                                        			\
		name##_() {                                         			\
			map4[#name]=&name;                              			\
			AddSysFuncDecl(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE, t4##_TYPE }, waitable, dev); \
		}                                                   			\
	} name##_instance

#define RegFunc5(name, ret, t1, t2, t3, t4, t5, waitable, dev)                      \
	Value name(const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5) {          			\
		return name(x1.to<t1>(), x2.to<t2>(), x3.to<t3>(), x4.to<t4>(), x5.to<t5>());              			\
	}                                                       			\
	struct name##_ {                                        			\
		name##_() {                                         			\
			map5[#name]=&name;                              			\
			AddSysFuncDecl(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE, t4##_TYPE, t5##_TYPE }, waitable, dev); \
		}                                                   			\
	} name##_instance

#define shpvoid int

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

shpvoid SetEgo(std::string actorName);
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
shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex);
shpvoid WalkerBoundaryUnblockModel(std::string modelName);
shpvoid WalkerBoundaryUnblockRegion(int regionIndex, int regionBoundaryIndex);

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

shpvoid SetDefaultDialogueCamera(std::string cameraName);

shpvoid StartAnimation(std::string animationName); // WAIT
shpvoid StartMoveAnimation(std::string animName); // WAIT
shpvoid StartMom(std::string momAnimationName); // WAIT
shpvoid LoopAnimation(std::string animationName);
shpvoid StopAnimation(std::string animationName);
shpvoid StopAllAnimations(); // DEV

shpvoid StartMorphAnimation(std::string animationName, int animStartFrame, int morphFrames); // WAIT
shpvoid StopMorphAnimation(std::string animationName);
shpvoid StopAllMorphAnimations(); // DEV

shpvoid StartVoiceOver(std::string dialogueName, int numLines); // WAIT
shpvoid StartYak(std::string yakAnimName); // WAIT, DEV

// APPLICATION
shpvoid AddPath(std::string pathName); // DEV
shpvoid FullScanPaths(); // DEV
shpvoid RescanPaths(); // DEV

shpvoid DumpBuildInfo(); // DEV
shpvoid DumpLayerStack(); // DEV

shpvoid Edit(std::string filename); // DEV
shpvoid Open(std::string filename); // DEV

shpvoid ForceQuitGame(); // DEV
shpvoid QuitApp(); // DEV

shpvoid FullReset(); // DEV

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
shpvoid ForceCutToCameraAngle(std::string cameraName);

shpvoid DefaultInspect(std::string noun); // WAIT

shpvoid EnableCameraBoundaries(); // DEV
shpvoid DisableCameraBoundaries(); // DEV

shpvoid DumpCamera(); // DEV
shpvoid DumpCameraAngles(); // DEV

int GetCameraAngleCount(); // DEV
std::string GetIndexedCameraAngle(int index); // DEV

shpvoid SetCameraGlide(int glide);
shpvoid GlideToCameraAngle(std::string cameraName); // WAIT

shpvoid InspectObject(); // WAIT
shpvoid Uninspect(); // WAIT

float GetCameraFOV();
shpvoid SetCameraFOV(float fov);

// CONSTRUCTION MODE
shpvoid ShowConstruction(); // DEV
shpvoid HideConstruction(); // DEV

std::string CreateCameraAngleGizmo(); // DEV
std::string CreateCameraAngleGizmoX(float horizAngle, float vertAngle,
                                    float x, float y, float z); // DEV
shpvoid ShowCameraAngleGizmo(std::string cameraName); // DEV
shpvoid HideCameraAngleGizmo(std::string cameraName); // DEV

std::string CreatePositionGizmo(); // DEV
std::string CreatePositionGizmoX(float heading, float x, float y, float z); // DEV
shpvoid ShowPositionGizmo(std::string positionName); // DEV
shpvoid HidePositionGizmo(std::string positionName); // DEV

shpvoid ShowAmbientMapGizmo(); // DEV
shpvoid HideAmbientMapGizmo(); // DEV

shpvoid ShowWalkerBoundaryGizmo(); // DEV
shpvoid HideWalkerBoundaryGizmo(); // DEV

shpvoid SetSceneViewport(int xPercent, int yPercent, int widthPercent, int heightPercent); // DEV

shpvoid TextInspectCameraGizmo(std::string cameraName); // DEV
shpvoid TextInspectCameraGizmoX(std::string cameraName, int xPercent,
                                int yPercent, int fontSize); // DEV

shpvoid TextInspectPositionGizmo(std::string positionName); // DEV
shpvoid TextInspectPositionGizmoX(std::string positionName, int xPercent,
                                int yPercent, int fontSize); // DEV

shpvoid ViewportInspectCameraGizmo(std::string cameraName); // DEV
shpvoid ViewportInspectCameraGizmoX(std::string cameraName, int xPercent,
                                    int yPercent, int widthPercent, int heightPercent); // DEV

// DEBUGGING (ALL DEV)
shpvoid AddTemplate(std::string templateText, std::string expandedText, int removeTemplate);
shpvoid RemoveTemplate(std::string templateText);

shpvoid Alias(std::string alias, std::string sheepCommand);
shpvoid Unalias(std::string alias);

shpvoid BindDebugKey(std::string keyName, std::string sheepCommand);
shpvoid UnbindDebugKey(std::string keyName);

shpvoid OpenConsole();
shpvoid CloseConsole();
shpvoid ToggleConsole();

shpvoid SetConsole(std::string command);
shpvoid InsertConsole(std::string command);
shpvoid ClearConsole();
shpvoid ClearConsoleBuffer();

int GetDebugFlag(std::string flagName);
shpvoid SetDebugFlag(std::string flagName);
shpvoid ClearDebugFlag(std::string flagName);
shpvoid ToggleDebugFlag(std::string flagName);

shpvoid DumpDebugFlags();
shpvoid DumpFile(std::string filename);
shpvoid DumpLockedObjects();
shpvoid DumpMemoryUsage();
shpvoid DumpPathFileMap();
shpvoid DumpUsedPaths();
shpvoid DumpUsedFiles();

shpvoid ReportMemoryUsage();
shpvoid ReportSurfaceMemoryUsage();

float GetTimeMultiplier();
shpvoid SetTimeMultiplier(float multiplier);

// ENGINE
shpvoid Call(std::string functionName); // WAIT
shpvoid CallDefaultSheep(std::string sheepFileName); // WAIT
shpvoid CallSheep(std::string sheepFileName, std::string functionName); // WAIT

shpvoid CallGlobal(std::string functionName); // WAIT
shpvoid CallGlobalSheep(std::string sheepFileName, std::string functionName); // WAIT

shpvoid CallIndexedSheep(std::string sheepFileName, int sheepIndex, std::string functionName); // DEV, WAIT

shpvoid EnableCinematics(); // DEV
shpvoid DisableCinematics(); // DEV

shpvoid EnableIncrementalRendering(); // DEV
shpvoid DisableIncrementalRendering(); // DEV

shpvoid EnableSheepCaching(); // DEV
shpvoid DisableSheepCaching(); // DEV
shpvoid DisableCurrentSheepCaching(); // DEV

shpvoid Extract(std::string fileSpec, std::string outputPath);

shpvoid DumpActiveSheepObjects(); // DEV
shpvoid DumpActiveSheepThreads(); // DEV
shpvoid DumpCommands(); // DEV
shpvoid DumpRawSheep(std::string sheepName); // DEV
shpvoid DumpSheepEngine(); // DEV

shpvoid ExecCommand(std::string sheepCommand); // DEV, WAIT
shpvoid FindCommand(std::string commandGuess); // DEV
shpvoid HelpCommand(std::string commandName); // DEV

std::string GetCurrentSheepFunction();
std::string GetCurrentSheepName();

shpvoid SetGlobalSheep();
shpvoid SetTopSheep();

shpvoid NukeAllSheep(); // DEV
shpvoid NukeSheep(std::string sheepName); // DEV

float GetGamma(); // DEV
shpvoid SetGamma(float gamma); // DEV

shpvoid SaveSprite(std::string spriteName, std::string fileName); // DEV
shpvoid SaveTexture(std::string textureName, std::string fileName); // DEV
shpvoid SaveTextureX(std::string textureName, int surfaceIndex, std::string fileName); // DEV

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

int GetFlag(std::string flagName);
int GetFlagInt(int flagEnum);
shpvoid SetFlag(std::string flagName);
shpvoid ClearFlag(std::string flagName);

shpvoid DumpFlags(); // DEV
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
shpvoid IncNounVerbCountBoth(std::string noun, std::string verb);
shpvoid SetNounVerbCount(std::string noun, std::string verb, int count);
shpvoid SetNounVerbCountBoth(std::string noun, std::string verb, int count);
shpvoid TriggerNounVerb(std::string noun, std::string verb); // DEV

int GetScore();
shpvoid IncreaseScore(int value);
shpvoid SetScore(int score); // DEV
shpvoid ChangeScore(std::string scoreValue);

int GetTopicCount(std::string noun, std::string verb);
int GetTopicCountInt(int nounEnum, int verbEnum);
int HasTopicsLeft(std::string noun);
shpvoid SetTopicCount(std::string noun, std::string verb, int count); // DEV

int IsCurrentLocation(std::string location);
int IsCurrentTime(std::string timeblock);
int WasLastLocation(std::string location);
int WasLastTime(std::string timeblock);

shpvoid ResetGameData(); // DEV

shpvoid SetGameTimer(std::string noun, std::string verb, int milliseconds);

// GENERAL
shpvoid DrawFilledRect(int left, int top, int right, int bottom,
                       int red, int green, int blue); // DEV

shpvoid FollowOnDrivingMap(int followState); // WAIT

float GetRandomFloat(float lower, float upper);
int GetRandomInt(int lower, int upper);

shpvoid PlayFullScreenMovie(std::string movieName); // WAIT
shpvoid PlayMovie(std::string movieName); // WAIT

shpvoid SetPamphletPage(int page);

// INSETS
shpvoid DumpInsetNames(); // DEV

shpvoid ShowInset(std::string insetName);
shpvoid HideInset();

shpvoid ShowPlate(std::string plateName);
shpvoid HidePlate(std::string plateName);

// INVENTORY
shpvoid CombineInvItems(std::string firstItemName, std::string secondItemName,
                        std::string combinedItemName);

int DoesEgoHaveInvItem(std::string itemName);
int DoesGabeHaveInvItem(std::string itemName);
int DoesGraceHaveInvItem(std::string itemName);
shpvoid EgoTakeInvItem(std::string itemName);

shpvoid DumpEgoActiveInvItem(); // DEV
shpvoid SetEgoActiveInvItem(std::string itemName);

shpvoid ShowInventory();
shpvoid HideInventory();

shpvoid InventoryInspect(std::string itemName);
shpvoid InventoryUninspect();

shpvoid SetInvItemStatus(std::string itemName, std::string status);

// MODELS
shpvoid SetModelShadowTexture(std::string modelName, std::string textureName);
shpvoid ClearModelShadowTexture(std::string modelName);

shpvoid SetPropGas(std::string modelName, std::string gasName);
shpvoid ClearPropGas(std::string modelName);

int DoesModelExist(std::string modelName);
int DoesSceneModelExist(std::string modelName);

shpvoid DumpModel(std::string modelName); // DEV
shpvoid DumpModelNames(); // DEV
shpvoid DumpSceneModelNames(); // DEV

shpvoid ShowModel(std::string modelName);
shpvoid HideModel(std::string modelName);

shpvoid ShowModelGroup(std::string groupName);
shpvoid HideModelGroup(std::string groupName);

shpvoid ShowSceneModel(std::string modelName);
shpvoid HideSceneModel(std::string modelName);

int IsModelVisible(std::string modelName);
int IsSceneModelVisible(std::string modelName);

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
shpvoid SetLocationTime(std::string location, std::string timeblock); // WAIT
shpvoid SetTime(std::string timeblock); // WAIT

shpvoid SetScene(std::string sceneName); // WAIT

shpvoid Warp(std::string locationAndTime); // WAIT

// SOUND
shpvoid EnableSound(std::string soundType); // DEV
shpvoid DisableSound(std::string soundType); // DEV

shpvoid GetVolume(std::string soundType); // DEV
shpvoid SetVolume(std::string soundType); // DEV

shpvoid PlaySound(std::string soundName); // WAIT
shpvoid StopSound(std::string soundName);
shpvoid StopAllSounds();

shpvoid PlaySoundTrack(std::string soundtrackName); // WAIT
shpvoid StopSoundTrack(std::string soundtrackName); // WAIT
shpvoid StopAllSoundTracks();

// TRACING (ALL DEV)
shpvoid PrintFloat(float value);
shpvoid PrintFloatX(std::string category, float value);

shpvoid PrintInt(int value);
shpvoid PrintIntX(std::string category, int value);

shpvoid PrintIntHex(int value);
shpvoid PrintIntHexX(std::string category, int value);

shpvoid PrintString(std::string message);
shpvoid PrintStringX(std::string category, std::string message);

// UNDOCUMENTED
int IsTopLayerInventory();
