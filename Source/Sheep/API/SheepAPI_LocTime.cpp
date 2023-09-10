#include "SheepAPI_LocTime.h"

#include "CharacterManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "LocationManager.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "TimeblockScreen.h"
#include "VideoPlayer.h"

using namespace std;

int GetEgoCurrentLocationCount()
{
    return GetEgoLocationCount(gLocationManager.GetLocation());
}
RegFunc0(GetEgoCurrentLocationCount, int, IMMEDIATE, REL_FUNC);

int GetEgoLocationCount(const std::string& locationName)
{
    // Make sure it's a valid location.
    if(!gLocationManager.IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    // Get it!
    return gLocationManager.GetLocationCountForCurrentTimeblock(Scene::GetEgoName(), locationName);
}
RegFunc1(GetEgoLocationCount, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetEgoLocationCount(const std::string& locationName, int count)
{
    // Make sure it's a valid location.
    if(!gLocationManager.IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    // Set it!
    gLocationManager.SetLocationCountForCurrentTimeblock(Scene::GetEgoName(), locationName, count);
    return 0;
}
RegFunc2(SetEgoLocationCount, void, string, int, IMMEDIATE, DEV_FUNC);

int WasEgoEverInLocation(const std::string& locationName)
{
    // Make sure it's a valid location.
    if(!gLocationManager.IsValidLocation(locationName))
    {
        gReportManager.Log("Error", "Error: '" + locationName + "' is not a valid location name. Call DumpLocations() to see valid locations.");
        ExecError();
        return 0;
    }

    // Returns if Ego was EVER in a location during ANY timeblock!
    const std::string& egoName = Scene::GetEgoName();
    int locationCount = gLocationManager.GetLocationCountAcrossAllTimeblocks(egoName, locationName);
    return locationCount > 0 ? 1 : 0;
}
RegFunc1(WasEgoEverInLocation, int, string, IMMEDIATE, REL_FUNC);

int IsActorAtLocation(const std::string& actorName, const std::string& locationName)
{
    // Validate actor name.
    if(!gCharacterManager.IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    // Validate location.
    bool locationValid = gLocationManager.IsValidLocation(locationName);
    if(!locationValid)
    {
        ExecError();
        return 0;
    }

    // See if actor location matches specified location.
    return gLocationManager.IsActorAtLocation(actorName, locationName) ? 1 : 0;
}
RegFunc2(IsActorAtLocation, int, string, string, IMMEDIATE, REL_FUNC);

int IsActorOffstage(const std::string& actorName)
{
    // Validate actor name.
    if(!gCharacterManager.IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    return gLocationManager.IsActorOffstage(actorName) ? 1 : 0;
}
RegFunc1(IsActorOffstage, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorLocation(const std::string& actorName, const std::string& locationName)
{
    // Validate actor name.
    if(!gCharacterManager.IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    // Validate location.
    if(!gLocationManager.IsValidLocation(locationName))
    {
        ExecError();
        return 0;
    }

    gLocationManager.SetActorLocation(actorName, locationName);
    return 0;
}
RegFunc2(SetActorLocation, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorOffstage(const std::string& actorName)
{
    // Validate actor name.
    if(!gCharacterManager.IsValidName(actorName))
    {
        ExecError();
        return 0;
    }

    gLocationManager.SetActorOffstage(actorName);
    return 0;
}
RegFunc1(SetActorOffstage, void, string, IMMEDIATE, REL_FUNC);

int IsCurrentLocation(const std::string& location)
{
    std::string currentLocation = gLocationManager.GetLocation();
    return StringUtil::EqualsIgnoreCase(currentLocation, location) ? 1 : 0;
}
RegFunc1(IsCurrentLocation, int, string, IMMEDIATE, REL_FUNC);

int WasLastLocation(const std::string& location)
{
    std::string lastLocation = gLocationManager.GetLastLocation();
    return StringUtil::EqualsIgnoreCase(lastLocation, location) ? 1 : 0;
}
RegFunc1(WasLastLocation, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetLocation(const std::string& location)
{
    gLocationManager.ChangeLocation(location, AddWait());
    return 0;
}
RegFunc1(SetLocation, void, string, WAITABLE, REL_FUNC);

int IsCurrentTime(const std::string& timeblock)
{
    std::string currentTimeblock = gGameProgress.GetTimeblock().ToString();
    return StringUtil::EqualsIgnoreCase(currentTimeblock, timeblock) ? 1 : 0;
}
RegFunc1(IsCurrentTime, int, string, IMMEDIATE, REL_FUNC);

int WasLastTime(const std::string& timeblock)
{
    std::string lastTimeblock = gGameProgress.GetLastTimeblock().ToString();
    return StringUtil::EqualsIgnoreCase(lastTimeblock, timeblock) ? 1 : 0;
}
RegFunc1(WasLastTime, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetTime(const std::string& timeblock)
{
    // If already in the desired timeblock, we do nothing.
    Timeblock oldTimeblock = gGameProgress.GetTimeblock();
    Timeblock newTimeblock(timeblock);
    if(oldTimeblock == newTimeblock)
    {
        return 0;
    }

    // Report that a timeblock change IS occurring (though it hasn't fully happened yet).
    gGameProgress.SetChangingTimeblock(true);
    
    // Unload the current scene.
    std::function<void()> waitable = AddWait();
    gSceneManager.UnloadScene([oldTimeblock, newTimeblock, waitable](){
        
        // Play ending movie (if any) for the timeblock we are leaving.
        gVideoPlayer.Play(oldTimeblock.ToString() + "end", true, true, [newTimeblock, waitable](){

            // Show timeblock screen.
            gGK3UI.ShowTimeblockScreen(newTimeblock, 0.0f, [newTimeblock, waitable](){

                // Change time to new timeblock.
                gGameProgress.SetTimeblock(newTimeblock);

                // Show beginning movie (if any) for the new timeblock.
                gVideoPlayer.Play(newTimeblock.ToString() + "begin", true, true, [waitable](){

                    // Reload our current location in the new timeblock.
                    gSceneManager.LoadScene(gLocationManager.GetLocation());

                    // Done changing timeblock.
                    gGameProgress.SetChangingTimeblock(false);

                    // Notify waitable 'cause we are done here.
                    if(waitable != nullptr)
                    {
                        waitable();
                    }
                });
            });
        });
    });
    return 0;
}
RegFunc1(SetTime, void, string, WAITABLE, REL_FUNC);

shpvoid SetLocationTime(const std::string& location, const std::string& timeblock)
{
    // Set the location, but do not actually go to that scene yet.
    gLocationManager.SetLocation(location);

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