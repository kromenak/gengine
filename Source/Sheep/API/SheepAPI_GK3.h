//
// Clark Kromenaker
//
// Sheep SysFuncs that are very specific to GK3.
//
#pragma once
#include "SheepSysFunc.h"

shpvoid StartGame(); // DEV
shpvoid ForceQuitGame(); // DEV
shpvoid QuitApp(); // DEV

shpvoid ShowBinocs();
shpvoid ShowFingerprintInterface(const std::string& nounName);

shpvoid ShowSidney();
int DoesSidneyFileExist(const std::string& fileName);

shpvoid ShowDrivingInterface();
shpvoid FollowOnDrivingMap(int followState); // WAIT

shpvoid SetPamphletPage(int page);

shpvoid TurnLSRPageLeft();
shpvoid TurnLSRPageRight();
