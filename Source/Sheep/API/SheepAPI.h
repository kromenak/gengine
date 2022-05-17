//
// Clark Kromenaker
//
// Declares all Sheep SysFuncs that can be called from SheepScripts at runtime.
//
#pragma once
#include "SheepScript.h"
#include "SheepSysFunc.h"

#include "SheepAPI_Actors.h"
#include "SheepAPI_Animation.h"
#include "SheepAPI_Assets.h"
#include "SheepAPI_Camera.h"
#include "SheepAPI_Debug.h"
#include "SheepAPI_Dialogue.h"
#include "SheepAPI_Inventory.h"
#include "SheepAPI_LocTime.h"
#include "SheepAPI_Reports.h"
#include "SheepAPI_Scene.h"
#include "SheepAPI_Sound.h"

// SHEEP
shpvoid Call(const std::string& functionName); // WAIT
shpvoid CallDefaultSheep(const std::string& sheepFileName); // WAIT
shpvoid CallSheep(const std::string& sheepFileName, const std::string& functionName); // WAIT

shpvoid CallGlobal(const std::string& functionName); // WAIT
shpvoid CallGlobalSheep(const std::string& sheepFileName, const std::string& functionName); // WAIT

shpvoid CallIndexedSheep(const std::string& sheepFileName, int sheepIndex, const std::string& functionName); // DEV, WAIT

shpvoid NukeAllSheep(); // DEV
shpvoid NukeSheep(const std::string& sheepName); // DEV

shpvoid SetGlobalSheep();
shpvoid SetTopSheep();

shpvoid EnableSheepCaching(); // DEV
shpvoid DisableSheepCaching(); // DEV
shpvoid DisableCurrentSheepCaching(); // DEV

std::string GetCurrentSheepFunction();
std::string GetCurrentSheepName();

shpvoid ExecCommand(const std::string& sheepCommand); // DEV, WAIT
shpvoid FindCommand(const std::string& commandGuess); // DEV
shpvoid HelpCommand(const std::string& commandName); // DEV

shpvoid DumpActiveSheepObjects(); // DEV
shpvoid DumpActiveSheepThreads(); // DEV
shpvoid DumpCommands(); // DEV
shpvoid DumpRawSheep(const std::string& sheepName); // DEV
shpvoid DumpSheepEngine(); // DEV

// GK3
shpvoid StartGame(); // DEV

shpvoid ShowBinocs();
shpvoid ShowFingerprintInterface(std::string nounName);
shpvoid ShowSidney();

shpvoid ShowDrivingInterface();
shpvoid FollowOnDrivingMap(int followState); // WAIT

shpvoid SetPamphletPage(int page);

shpvoid TurnLSRPageLeft();
shpvoid TurnLSRPageRight();

// APPLICATION
shpvoid DumpBuildInfo(); // DEV

shpvoid ForceQuitGame(); // DEV
shpvoid QuitApp(); // DEV

float GetTimeMultiplier();
shpvoid SetTimeMultiplier(float multiplier);

float GetRandomFloat(float lower, float upper);
int GetRandomInt(int lower, int upper);

// RENDERING
shpvoid EnableIncrementalRendering(); // DEV
shpvoid DisableIncrementalRendering(); // DEV

float GetGamma(); // DEV
shpvoid SetGamma(float gamma); // DEV

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

shpvoid DrawFilledRect(int left, int top, int right, int bottom,
                       int red, int green, int blue); // DEV

shpvoid RefreshScreen(); // DEV

shpvoid Screenshot();
shpvoid ScreenshotX(std::string filename); // DEV

// VIDEO
shpvoid PlayFullScreenMovie(std::string movieName); // WAIT
shpvoid PlayFullScreenMovieX(std::string movieName, int autoclose); // WAIT
shpvoid PlayMovie(std::string movieName); // WAIT

// GAME LOGIC
int GetFlag(const std::string& flagName);
int GetFlagInt(int flagEnum);
shpvoid SetFlag(const std::string& flagName);
shpvoid ClearFlag(const std::string& flagName);
shpvoid DumpFlags(); // DEV

int GetGameVariableInt(const std::string& varName);
shpvoid IncGameVariableInt(const std::string& varName);
shpvoid SetGameVariableInt(const std::string& varName, int value);

int GetNounVerbCount(const std::string& noun, const std::string& verb);
int GetNounVerbCountInt(int nounEnum, int verbEnum);
shpvoid IncNounVerbCount(const std::string& noun, const std::string& verb);
shpvoid IncNounVerbCountBoth(const std::string& noun, const std::string& verb);
shpvoid SetNounVerbCount(const std::string& noun, const std::string& verb, int count);
shpvoid SetNounVerbCountBoth(const std::string& noun, const std::string& verb, int count);
shpvoid TriggerNounVerb(const std::string& noun, const std::string& verb); // DEV

int GetScore();
shpvoid IncreaseScore(int value);
shpvoid SetScore(int score); // DEV
shpvoid ChangeScore(const std::string& scoreValue);

int GetTopicCount(const std::string& noun, const std::string& verb);
int GetTopicCountInt(int nounEnum, int verbEnum);
int HasTopicsLeft(const std::string& noun);
shpvoid SetTopicCount(std::string noun, std::string verb, int count); // DEV

int GetChatCount(const std::string& noun);
int GetChatCountInt(int nounEnum);
shpvoid SetChatCount(std::string noun, int count); // DEV

shpvoid FullReset(); // DEV
shpvoid ResetGameData(); // DEV

shpvoid SetGameTimer(std::string noun, std::string verb, int milliseconds);
shpvoid SetTimerMs(int milliseconds); // WAIT
shpvoid SetTimerSeconds(float seconds); // WAIT