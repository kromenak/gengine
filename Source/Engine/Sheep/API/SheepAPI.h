//
// Clark Kromenaker
//
// Core Sheep SysFuncs for manipulating the Sheep system itself.
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
#include "SheepAPI_GameLogic.h"
#include "SheepAPI_GK3.h"
#include "SheepAPI_Inventory.h"
#include "SheepAPI_LocTime.h"
#include "SheepAPI_Rendering.h"
#include "SheepAPI_Reports.h"
#include "SheepAPI_Scene.h"
#include "SheepAPI_Sound.h"

shpvoid Call(const std::string& functionName); // WAIT
shpvoid CallDefaultSheep(const std::string& sheepFileName); // WAIT
shpvoid CallSheep(const std::string& sheepFileName, const std::string& functionName); // WAIT

shpvoid CallGlobal(const std::string& functionName); // WAIT
shpvoid CallGlobalSheep(const std::string& sheepFileName, const std::string& functionName); // WAIT

shpvoid CallIndexedSheep(const std::string& sheepFileName, int sheepIndex, const std::string& functionName); // DEV, WAIT

shpvoid SetGlobalSheep();
shpvoid SetTopSheep();

shpvoid NukeAllSheep(); // DEV
shpvoid NukeSheep(const std::string& sheepName); // DEV

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
