#include "SheepAPI_LocTime.h"

#include "CharacterManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "LocationManager.h"
#include "Scene.h"
#include "Services.h"
#include "TimeblockScreen.h"

using namespace std;

int GetEgoCurrentLocationCount()
{
    return GetEgoLocationCount(Services::Get<LocationManager>()->GetLocation());
}
RegFunc0(GetEgoCurrentLocationCount, int, IMMEDIATE, REL_FUNC);

int GetEgoLocationCount(const std::string& locationName)
{
    // Make sure it's a valid location.
    if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    // Get it!
    return Services::Get<LocationManager>()->GetLocationCountForCurrentTimeblock(Scene::GetEgoName(), locationName);
}
RegFunc1(GetEgoLocationCount, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetEgoLocationCount(const std::string& locationName, int count)
{
    // Make sure it's a valid location.
    if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    // Set it!
    Services::Get<LocationManager>()->SetLocationCountForCurrentTimeblock(Scene::GetEgoName(), locationName, count);
    return 0;
}
RegFunc2(SetEgoLocationCount, void, string, int, IMMEDIATE, DEV_FUNC);

int WasEgoEverInLocation(const std::string& locationName)
{
    // Make sure it's a valid location.
    if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
    {
        Services::GetReports()->Log("Error", "Error: '" + locationName + "' is not a valid location name. Call DumpLocations() to see valid locations.");
        ExecError();
        return 0;
    }

    // Returns if Ego was EVER in a location during ANY timeblock!
    const std::string& egoName = Scene::GetEgoName();
    int locationCount = Services::Get<LocationManager>()->GetLocationCountAcrossAllTimeblocks(egoName, locationName);
    return locationCount > 0 ? 1 : 0;
}
RegFunc1(WasEgoEverInLocation, int, string, IMMEDIATE, REL_FUNC);

int IsActorAtLocation(const std::string& actorName, const std::string& locationName)
{
    // Validate actor name.
    if(!Services::Get<CharacterManager>()->IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    // Validate location.
    bool locationValid = Services::Get<LocationManager>()->IsValidLocation(locationName);
    if(!locationValid)
    {
        ExecError();
        return 0;
    }

    // See if actor location matches specified location.
    return Services::Get<LocationManager>()->IsActorAtLocation(actorName, locationName) ? 1 : 0;
}
RegFunc2(IsActorAtLocation, int, string, string, IMMEDIATE, REL_FUNC);

int IsActorOffstage(const std::string& actorName)
{
    // Validate actor name.
    if(!Services::Get<CharacterManager>()->IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    return Services::Get<LocationManager>()->IsActorOffstage(actorName) ? 1 : 0;
}
RegFunc1(IsActorOffstage, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorLocation(const std::string& actorName, const std::string& locationName)
{
    // Validate actor name.
    if(!Services::Get<CharacterManager>()->IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    // Validate location.
    if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    Services::Get<LocationManager>()->SetActorLocation(actorName, locationName);
    return 0;
}
RegFunc2(SetActorLocation, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorOffstage(const std::string& actorName)
{
    // Validate actor name.
    if(!Services::Get<CharacterManager>()->IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    Services::Get<LocationManager>()->SetActorOffstage(actorName);
    return 0;
}
RegFunc1(SetActorOffstage, void, string, IMMEDIATE, REL_FUNC);

int IsCurrentLocation(const std::string& location)
{
    std::string currentLocation = Services::Get<LocationManager>()->GetLocation();
    return StringUtil::EqualsIgnoreCase(currentLocation, location) ? 1 : 0;
}
RegFunc1(IsCurrentLocation, int, string, IMMEDIATE, REL_FUNC);

int WasLastLocation(const std::string& location)
{
    std::string lastLocation = Services::Get<LocationManager>()->GetLastLocation();
    return StringUtil::EqualsIgnoreCase(lastLocation, location) ? 1 : 0;
}
RegFunc1(WasLastLocation, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetLocation(const std::string& location)
{
    Services::Get<LocationManager>()->ChangeLocation(location);
    return 0;
}
RegFunc1(SetLocation, void, string, WAITABLE, REL_FUNC);

int IsCurrentTime(const std::string& timeblock)
{
    std::string currentTimeblock = Services::Get<GameProgress>()->GetTimeblock().ToString();
    return StringUtil::EqualsIgnoreCase(currentTimeblock, timeblock) ? 1 : 0;
}
RegFunc1(IsCurrentTime, int, string, IMMEDIATE, REL_FUNC);

int WasLastTime(const std::string& timeblock)
{
    std::string lastTimeblock = Services::Get<GameProgress>()->GetLastTimeblock().ToString();
    return StringUtil::EqualsIgnoreCase(lastTimeblock, timeblock) ? 1 : 0;
}
RegFunc1(WasLastTime, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetTime(const std::string& timeblock)
{
    // If already in the desired timeblock, we do nothing.
    Timeblock newTimeblock(timeblock);
    if(Services::Get<GameProgress>()->GetTimeblock() == newTimeblock)
    {
        return 0;
    }

    // Change time to new timeblock.
    Services::Get<GameProgress>()->SetTimeblock(Timeblock(timeblock));
    
    // Unload the current scene.
    GEngine::Instance()->UnloadScene();

    // Show timeblock screen.
    gGK3UI.ShowTimeblockScreen(timeblock, 0.0f, []() {
        // We're assuming that just the time changed here, so load back into same location.
        GEngine::Instance()->LoadScene(Services::Get<LocationManager>()->GetLocation());
    });
    return 0;
}
RegFunc1(SetTime, void, string, WAITABLE, REL_FUNC);

shpvoid SetLocationTime(const std::string& location, const std::string& timeblock)
{
    // Set the location, but do not actually go to that scene yet.
    Services::Get<LocationManager>()->SetLocation(location);

    // Set the time. This will show the timeblock screen and actually load the location.
    SetTime(timeblock);
    return 0;
}
RegFunc2(SetLocationTime, void, string, string, WAITABLE, REL_FUNC);

/*
shpvoid Warp(std::string locationAndTime)
{
    // Lots of input handling to figure out string and handle errors...
    // Wonder if this is ever used?
    std::cout << "Warp" << std::endl;
    return 0;
}
RegFunc1(Warp, void, string, WAITABLE, REL_FUNC);
*/