#include "SheepAPI.h"

#include <functional> // for std::hash
#include <sstream> // for int->hex

#include "Animator.h"
#include "BSPActor.h"
#include "Camera.h"
#include "CharacterManager.h"
#include "DialogueManager.h"
#include "FaceController.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GKActor.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Random.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"
#include "TimeblockScreen.h"
#include "Timers.h"
#include "VerbManager.h"
#include "VideoPlayer.h"
#include "WalkerBoundary.h"

// Required for macros to work correctly with "string" instead of "std::string".
using namespace std;






/*
shpvoid LookitActor(std::string actorName, std::string otherActorName,
					 std::string componentsSpec, float durationSec)
{
	return 0;
}
RegFunc4(LookitActor, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitActorQuick(std::string actorName, std::string otherActorName,
						 std::string componentsSpec, float durationSec)
{
	return 0;
}
RegFunc4(LookitActorQuick, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModel(std::string actorName, std::string modelName,
					std::string componentsSpec, float durationSec)
{
	return 0;
}
RegFunc4(LookitModel, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModelQuick(std::string actorName, std::string modelName,
						 std::string componentsSpec, float durationSec)
{
	return 0;
}
RegFunc4(LookitModelQuick, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModelX(std::string actorName, std::string modelName, int mesh,
					 std::string boxModifier, float offsetX, float offsetY, float offsetZ,
					 std::string componentsSpec, float durationSec)
{
	return 0;
}
//TODO: Need a new RegFuncX for 9 arguments!

//LookitModelQuickX

//LookitNoun

//LookitNounQuick

//LookitPoint

//LookitCameraAngle

//LookitMouse

//LookitPlayer

//LookitCancel
*/

/*
//SetNextEgo

//SetPrevEgo

shpvoid SetWalkAnim(std::string actorName, std::string start, std::string cont,
					std::string startTurnLeft, std::string startTurnRight)
{
	std::cout << "SetWalkAnim" << std::endl;
	return 0;
}
RegFunc5(SetWalkAnim, void, string, string, string, string, string, IMMEDIATE, REL_FUNC);
*/

/*
shpvoid TurnHead(std::string actorName, int percentX, int percentY, int durationMs)
{
	std::cout << "TurnHead" << std::endl;
	return 0;
}
RegFunc4(TurnHead, void, string, int, int, int, WAITABLE, REL_FUNC);

shpvoid TurnToModel(std::string actorName, std::string modelName)
{
	std::cout << "TurnToModel" << std::endl;
	return 0;
}
RegFunc2(TurnToModel, void, string, string, WAITABLE, REL_FUNC);
*/

/*

shpvoid DumpBuildInfo()
{
	return 0;
}
RegFunc0(DumpBuildInfo, void, IMMEDIATE, DEV_FUNC);
*/

/*
shpvoid Edit(std::string filename)
{
	// Should open text editor defined in INI or default with file.
	return 0;
}
RegFunc1(Edit, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Open(std::string filename)
{
	// Tells OS to open a particular file.
	return 0;
}
RegFunc1(Open, void, string, IMMEDIATE, DEV_FUNC);
*/
 
shpvoid ForceQuitGame()
{
	GEngine::Instance()->Quit();
	return 0;
}
RegFunc0(ForceQuitGame, void, IMMEDIATE, DEV_FUNC);

shpvoid QuitApp()
{
	//TODO: Should quit nicely by asking "are you sure" and making sure you saved.
	GEngine::Instance()->Quit();
	return 0;
}
RegFunc0(QuitApp, void, IMMEDIATE, DEV_FUNC);

/*
shpvoid FullReset()
{
	// Clear caches, unload everything.
	// Re-enter current scene.
	return 0;
}
RegFunc0(FullReset, void, IMMEDIATE, DEV_FUNC);

//RefreshScreen

shpvoid Screenshot()
{
	std::cout << "Screenshot" << std::endl;
	return 0;
}
RegFunc0(Screenshot, void, IMMEDIATE, REL_FUNC);

shpvoid ScreenshotX(std::string filename)
{
	return 0;
}
RegFunc1(ScreenshotX, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ShowBinocs()
{
	return 0;
}
RegFunc0(ShowBinocs, void, IMMEDIATE, REL_FUNC);
*/

shpvoid ShowDrivingInterface()
{
    assert(false);
	return 0;
}
RegFunc0(ShowDrivingInterface, void, IMMEDIATE, REL_FUNC);

shpvoid ShowFingerprintInterface(std::string nounName)
{
    assert(false);
	return 0;
}
RegFunc1(ShowFingerprintInterface, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid ShowSidney()
{
	return 0;
}
RegFunc0(ShowSidney, void, IMMEDIATE, REL_FUNC);

shpvoid StartGame()
{
	// Same thing as hitting "Play" on title screen.
	return 0;
}
RegFunc0(StartGame, void, IMMEDIATE, DEV_FUNC);
*/
 

 



// CONSTRUCTION MODE
//ShowConstruction
//HideConstruction

//CreateCameraAngleGizmo
//CreateCameraAngleGizmoX
//ShowCameraAngleGizmo
//HideCameraAngleGizmo

//CreatePositionGizmo
//CreatePositionGizmoX
//ShowPositionGizmo
//HidePositionGizmo

//ShowAmbientMapGizmo
//HideAmbientMapGizmo

//ShowWalkerBoundaryGizmo
//HideWalkerBoundaryGizmo

//SetSceneViewport

//TextInspectCameraGizmo
//TextInspectCameraGizmoX

//TextInspectPositionGizmo
//TextInspectPositionGizmoX

//ViewportInspectCameraGizmo
//ViewportInspectCameraGizmoX

// DEBUGGING
//AddTemplate
//RemoveTemplate

//Alias
//Unalias

//BindDebugKey
//UnbindDebugKey

//OpenConsole
//CloseConsole
//ToggleConsole

//SetConsole
//InsertConsole
//ClearConsole
//ClearConsoleBuffer

//GetDebugFlag
//SetDebugFlag
//ClearDebugFlag
//ToggleDebugFlag

//DumpDebugFlags
//DumpFile
//DumpLockedObjects
//DumpMemoryUsage
//DumpPathFileMap
//DumpUsedPaths
//DumpUsedFiles

//ReportMemoryUsage
//ReportSurfaceMemoryUsage

//GetTimeMultiplier
//SetTimeMultiplier

// ENGINE
shpvoid Call(const std::string& functionName)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		SheepScript* sheep = currentThread->mContext->mSheepScript;
		if(sheep != nullptr)
		{
			Services::GetSheep()->Execute(sheep, functionName, currentThread != nullptr ? currentThread->AddWait() : nullptr);
		}
	}
	return 0;
}
RegFunc1(Call, void, string, WAITABLE, REL_FUNC);

shpvoid CallDefaultSheep(const std::string& fileName)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(fileName);
	if(script != nullptr)
	{
		SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
		Services::GetSheep()->Execute(script, currentThread != nullptr ? currentThread->AddWait() : nullptr);
	}
	return 0;
}
RegFunc1(CallDefaultSheep, void, string, WAITABLE, REL_FUNC);

shpvoid CallSheep(const std::string& fileName, const std::string& functionName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(!functionName.empty() && functionName.back() != '$')
	{
        // Make sure function name has the '$' suffix.
	    // Some GK3 data files do this, some don't!
        Services::GetSheep()->Execute(fileName, functionName + "$", currentThread != nullptr ? currentThread->AddWait() : nullptr);
	}
    else
    {
        Services::GetSheep()->Execute(fileName, functionName, currentThread != nullptr ? currentThread->AddWait() : nullptr);
    }
    return 0;
}
RegFunc2(CallSheep, void, string, string, WAITABLE, REL_FUNC);

/*
shpvoid CallGlobal(std::string functionName)
{
	std::cout << "CallGlobal" << std::endl;
	return 0;
}
RegFunc1(CallGlobal, void, string, WAITABLE, REL_FUNC);

shpvoid CallGlobalSheep(std::string sheepFileName, std::string functionName)
{
	std::cout << "CallGlobalSheep" << std::endl;
	return 0;
}
RegFunc2(CallGlobalSheep, void, string, string, WAITABLE, REL_FUNC);
*/

//EnableIncrementalRendering
//DisableIncrementalRendering

std::string GetCurrentSheepFunction()
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	return currentThread != nullptr ? currentThread->mFunctionName : "";
}
RegFunc0(GetCurrentSheepFunction, string, IMMEDIATE, REL_FUNC);

std::string GetCurrentSheepName()
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		// Specifically, return name with no extension.
		return currentThread->mContext->mSheepScript->GetNameNoExtension();
	}
	return "";
}
RegFunc0(GetCurrentSheepName, string, IMMEDIATE, REL_FUNC);

/*
shpvoid SetGlobalSheep()
{
	// Attach current sheep to global layer.
	// Means sheep stays active even when adding new game layers or changing scenes.
	std::cout << "SetGlobalSheep" << std::endl;
	return 0;
}
RegFunc0(SetGlobalSheep, void, IMMEDIATE, REL_FUNC);

shpvoid SetTopSheep()
{
	// Attach current sheep to top layer.
	// Essentially, undoes a SetGlobalSheep.
	// Sheep will stop on scene change.
	std::cout << "SetTopSheep" << std::endl;
	return 0;
}
RegFunc0(SetTopSheep, void, IMMEDIATE, REL_FUNC);
*/
 
//NukeAllSheep
//NukeSheep

//GetGamma
//SetGamma

//SaveSprite
//SaveTexture
//SaveTextureX

//SetRenderFlat
//SetRenderFull
//SetRenderShaded
//SetRenderWireframe

//SetShadowTypeBlobby
//SetShadowTypeModel
//SetShadowTypeNone

//SetSurfaceHigh
//SetSurfaceLow
//SetSurfaceNormal

shpvoid SetTimerMs(int milliseconds)
{
    // Should throw error if not waited upon!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    if(!currentThread->mInWaitBlock)
    {
        Services::GetReports()->Log("Warning", "No point setting a timer if you don't wait for it to finish. " +
                                                std::to_string(milliseconds) + " millisecond timer request ignored.");
        ExecError();
    }
    else
    {
        Timers::AddTimerMilliseconds(static_cast<unsigned int>(milliseconds), currentThread->AddWait());
    }
	return 0;
}
RegFunc1(SetTimerMs, void, int, WAITABLE, REL_FUNC);

shpvoid SetTimerSeconds(float seconds)
{
    // Should throw error if not waited upon!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    if(!currentThread->mInWaitBlock)
    {
        Services::GetReports()->Log("Warning", "No point setting a timer if you don't wait for it to finish. " +
                                    std::to_string(seconds) + " second timer request ignored.");
        ExecError();
    }
    else
    {
        Timers::AddTimerSeconds(seconds, currentThread->AddWait());
    }
    return 0;
}
RegFunc1(SetTimerSeconds, void, float, WAITABLE, REL_FUNC);
 
//ThrowException

//UnloadAll
//UnloadAllAnimations
//UnloadAllModels
//UnloadAllMovies
//UnloadAllScenes
//UnloadAllSounds
//UnloadAllSprites
//UnloadAllTextures

//UnloadAnimation
//UnloadModel
//UnloadMovie
//UnloadScene
//UnloadSound
//UnloadSprite
//UnloadTexture

// GAME LOGIC
//AddCaseLogic
//CheckCase
//CommitCaseLogic
//DumpCaseLogic
//ResetCaseLogic

int GetFlag(const std::string& flagName)
{
	return Services::Get<GameProgress>()->GetFlag(flagName);
}
RegFunc1(GetFlag, int, string, IMMEDIATE, REL_FUNC);

/*
int GetFlagInt(int flagEnum)
{
	// This function gets a flag, not by name, but by internal integer value.
	// I'm a bit unclear how any Sheep caller would know this value, and how
	// this value would be persistant across runs of the game...
	std::cout << "GetFlagInt was called!" << std::endl;
	return 0;
}
RegFunc1(GetFlagInt, int, int, IMMEDIATE, REL_FUNC);
*/
 
shpvoid SetFlag(const std::string& flagName)
{
	Services::Get<GameProgress>()->SetFlag(flagName);
	return 0;
}
RegFunc1(SetFlag, void, string, IMMEDIATE, REL_FUNC);

shpvoid ClearFlag(const std::string& flagName)
{
	Services::Get<GameProgress>()->ClearFlag(flagName);
	return 0;
}
RegFunc1(ClearFlag, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid DumpFlags()
{
	return 0;
}
RegFunc0(DumpFlags, void, IMMEDIATE, DEV_FUNC);
*/
 
//DumpNouns

int GetChatCount(const std::string& noun)
{
	return Services::Get<GameProgress>()->GetChatCount(noun);
}
RegFunc1(GetChatCount, int, string, IMMEDIATE, REL_FUNC);

int GetChatCountInt(int nounEnum)
{
    return GetChatCount(Services::Get<ActionManager>()->GetNoun(nounEnum));
}
RegFunc1(GetChatCountInt, int, int, IMMEDIATE, REL_FUNC);
 
shpvoid SetChatCount(std::string noun, int count)
{
	Services::Get<GameProgress>()->SetChatCount(noun, count);
	return 0;
}
RegFunc2(SetChatCount, void, string, int, IMMEDIATE, DEV_FUNC);

int GetGameVariableInt(const std::string& varName)
{
	return Services::Get<GameProgress>()->GetGameVariable(varName);
}
RegFunc1(GetGameVariableInt, int, string, IMMEDIATE, REL_FUNC);

shpvoid IncGameVariableInt(const std::string& varName)
{
	Services::Get<GameProgress>()->IncGameVariable(varName);
	return 0;
}
RegFunc1(IncGameVariableInt, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetGameVariableInt(const std::string& varName, int value)
{
	Services::Get<GameProgress>()->SetGameVariable(varName, value);
	return 0;
}
RegFunc2(SetGameVariableInt, void, string, int, IMMEDIATE, REL_FUNC);

int GetNounVerbCount(const std::string& noun, const std::string& verb)
{
	return Services::Get<GameProgress>()->GetNounVerbCount(noun, verb);
}
RegFunc2(GetNounVerbCount, int, string, string, IMMEDIATE, REL_FUNC);

int GetNounVerbCountInt(int nounEnum, int verbEnum)
{
    return GetNounVerbCount(Services::Get<ActionManager>()->GetNoun(nounEnum),
                            Services::Get<ActionManager>()->GetVerb(verbEnum));
}
RegFunc2(GetNounVerbCountInt, int, int, int, IMMEDIATE, REL_FUNC);
 
shpvoid IncNounVerbCount(const string& noun, const string& verb)
{
	//TODO: Throw an error if the given noun corresponds to a "Topic".
	Services::Get<GameProgress>()->IncNounVerbCount(noun, verb);
	return 0;
}
RegFunc2(IncNounVerbCount, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid IncNounVerbCountBoth(const string& noun, const string& verb)
{
    //TODO: Throw an error if the given noun corresponds to a "Topic".
    Services::Get<GameProgress>()->IncNounVerbCount("Gabriel", noun, verb);
	Services::Get<GameProgress>()->IncNounVerbCount("Grace", noun, verb);
	return 0;
}
RegFunc2(IncNounVerbCountBoth, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetNounVerbCount(const std::string& noun, const std::string& verb, int count)
{
	//TODO: Throw an error if the given noun corresponds to a "Topic".
	Services::Get<GameProgress>()->SetNounVerbCount(noun, verb, count);
	return 0;
}
RegFunc3(SetNounVerbCount, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid SetNounVerbCountBoth(const std::string& noun, const std::string& verb, int count)
{
    //TODO: Throw an error if the given noun corresponds to a "Topic".
    Services::Get<GameProgress>()->SetNounVerbCount("Gabriel", noun, verb, count);
    Services::Get<GameProgress>()->SetNounVerbCount("Grace", noun, verb, count);
    return 0;
}
RegFunc3(SetNounVerbCountBoth, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid TriggerNounVerb(const std::string& noun, const std::string& verb)
{
	//TODO: Validate noun or throw error.
	//TODO: Validate verb or throw error.
	bool success = Services::Get<ActionManager>()->ExecuteAction(noun, verb);
	if(!success)
	{
		Services::GetReports()->Log("Error", "Error: unable to trigger noun-verb combination " + noun + ":" + verb);
	}
	return 0;
}
RegFunc2(TriggerNounVerb, void, string, string, IMMEDIATE, DEV_FUNC);
 
int GetScore()
{
	return Services::Get<GameProgress>()->GetScore();
}
RegFunc0(GetScore, int, IMMEDIATE, REL_FUNC);

shpvoid IncreaseScore(int value)
{
	Services::Get<GameProgress>()->IncreaseScore(value);
	return 0;
}
RegFunc1(IncreaseScore, void, int, IMMEDIATE, REL_FUNC);

shpvoid SetScore(int score)
{
	Services::Get<GameProgress>()->SetScore(score);
	return 0;
}
RegFunc1(SetScore, void, int, IMMEDIATE, DEV_FUNC);

shpvoid ChangeScore(const std::string& scoreValue)
{
    Services::Get<GameProgress>()->ChangeScore(scoreValue);
	return 0;
}
RegFunc1(ChangeScore, void, string, IMMEDIATE, REL_FUNC);

int GetTopicCount(const std::string& noun, const std::string& verb)
{
	//TODO: Validate noun. Must be a valid noun. Seems to include any scene nouns, inventory nouns, actor nouns.
	if(!Services::Get<VerbManager>()->IsTopic(verb))
	{
		Services::GetReports()->Log("Error", "Error: '" + verb + " is not a valid verb name.");
		return 0;
	}
	return Services::Get<GameProgress>()->GetTopicCount(noun, verb);
}
RegFunc2(GetTopicCount, int, string, string, IMMEDIATE, REL_FUNC);

int GetTopicCountInt(int nounEnum, int verbEnum)
{
	std::string noun = Services::Get<ActionManager>()->GetNoun(nounEnum);
	std::string verb = Services::Get<ActionManager>()->GetVerb(verbEnum);
	return GetTopicCount(noun, verb);
}
RegFunc2(GetTopicCountInt, int, int, int, IMMEDIATE, REL_FUNC);
 
int HasTopicsLeft(const std::string& noun)
{
	//TODO: Validate noun.
	bool hasTopics = Services::Get<ActionManager>()->HasTopicsLeft(noun);
	return hasTopics ? 1 : 0;
}
RegFunc1(HasTopicsLeft, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetTopicCount(std::string noun, std::string verb, int count)
{
	//TODO: Validate noun or report error.
	//TODO: Validate verb or report error.
	Services::Get<GameProgress>()->SetTopicCount(noun, verb, count);
	return 0;
}
RegFunc3(SetTopicCount, void, string, string, int, IMMEDIATE, DEV_FUNC);
 
/*
shpvoid SetGameTimer(std::string noun, std::string verb, int milliseconds)
{
	std::cout << "SetGameTimer" << std::endl;
	return 0;
}
RegFunc3(SetGameTimer, void, string, string, int, IMMEDIATE, REL_FUNC);
*/
 
// GENERAL
//DrawFilledRect

shpvoid FollowOnDrivingMap(int followState)
{
	std::cout << "FollowOnDrivingMap" << std::endl;
	return 0;
}
RegFunc1(FollowOnDrivingMap, void, int, WAITABLE, REL_FUNC);

float GetRandomFloat(float lower, float upper)
{
	return Random::Range(lower, upper);
}
RegFunc2(GetRandomFloat, float, float, float, IMMEDIATE, REL_FUNC);

int GetRandomInt(int lower, int upper)
{
	// Both upper and lower are inclusive, according to docs.
	// So, add one to upper since our range function is upper-exclusive.
	return Random::Range(lower, upper + 1);
}
RegFunc2(GetRandomInt, int, int, int, IMMEDIATE, REL_FUNC);

shpvoid PlayFullScreenMovie(std::string movieName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, true, true, currentThread->AddWait());
	return 0;
}
RegFunc1(PlayFullScreenMovie, void, string, WAITABLE, REL_FUNC);

shpvoid PlayFullScreenMovieX(std::string movieName, int autoclose)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, true, autoclose != 0 ? true : false, currentThread->AddWait());
    return 0;
}
RegFunc2(PlayFullScreenMovieX, void, string, int, WAITABLE, REL_FUNC);

shpvoid PlayMovie(std::string movieName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, false, true, currentThread->AddWait());
	return 0;
}
RegFunc1(PlayMovie, void, string, WAITABLE, REL_FUNC);

/*
shpvoid SetPamphletPage(int page)
{
	std::cout << "SetPamphletPage" << std::endl;
	return 0;
}
RegFunc1(SetPamphletPage, void, int, WAITABLE, REL_FUNC);
*/

shpvoid TurnLSRPageLeft()
{
    assert(false);
    return 0;
}
RegFunc0(TurnLSRPageLeft, void, IMMEDIATE, REL_FUNC);

shpvoid TurnLSRPageRight()
{
    assert(false);
    return 0;
}
RegFunc0(TurnLSRPageRight, void, IMMEDIATE, REL_FUNC);