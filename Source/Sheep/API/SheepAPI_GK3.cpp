#include "SheepAPI_GK3.h"

#include "GEngine.h"
#include "GK3UI.h"
#include "Sidney.h"

using namespace std;

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

shpvoid ShowDrivingInterface()
{
    gGK3UI.ShowDrivingScreen();
    return 0;
}
RegFunc0(ShowDrivingInterface, void, IMMEDIATE, REL_FUNC);

shpvoid ShowFingerprintInterface(const std::string& nounName)
{
    assert(false);
    return 0;
}
RegFunc1(ShowFingerprintInterface, void, string, IMMEDIATE, REL_FUNC);

shpvoid ShowSidney()
{
    gGK3UI.ShowSidney();
    return 0;
}
RegFunc0(ShowSidney, void, IMMEDIATE, REL_FUNC);

int DoesSidneyFileExist(const std::string& fileName)
{
    bool exists = gGK3UI.GetSidney()->HasFile(fileName);
    return exists ? 1 : 0;
}
RegFunc1(DoesSidneyFileExist, int, string, IMMEDIATE, REL_FUNC);

shpvoid FollowOnDrivingMap(int followState)
{
    gGK3UI.ShowDrivingScreen(followState);
    return 0;
}
RegFunc1(FollowOnDrivingMap, void, int, WAITABLE, REL_FUNC);

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

shpvoid FinishedScreen()
{
    gGK3UI.ShowFinishedScreen();
    return 0;
}
RegFunc0(FinishedScreen, void, IMMEDIATE, REL_FUNC);