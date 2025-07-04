#include "LocationManager.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "IniParser.h"
#include "Localizer.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "SheepManager.h"
#include "StringUtil.h"
#include "TextAsset.h"
#include "Timeblock.h"

LocationManager gLocationManager;

void LocationManager::Init()
{
    // Parse as INI file.
    TextAsset* textFile = gAssetManager.LoadText("Locations.txt", AssetScope::Manual);
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    parser.ParseAll();

    IniSection locations = parser.GetSection("LOCATIONS");
    for(auto& line : locations.lines)
    {
        IniKeyValue& entry = line.entries.front();
        mLocCodeShortToLocCodeLong[entry.key] = entry.value;
    }
    delete textFile;

    //TEMP: Kind of handy to read in all BSPs at once - to test unknown values and such.
    /*
    std::vector<std::string> allBsp = {
        "ARM_A.BSP",
        "ARM_B.BSP",
        "B21.BSP",
        "B23.BSP",
        "B25.BSP",
        "B27.BSP",
        "B29.BSP",
        "B31.BSP",
        "BEC.BSP",
        "BET.BSP",
        "BMB.BSP",
        "CD1.BSP",
        "CDB.BSP",
        "CDB_E_N.BSP",
        "CEM.BSP",
        "CEM_A_E.BSP",
        "CEM_B_E.BSP",
        "CEM_C_E.BSP",
        "CEM_D_E.BSP",
        "CHU.BSP",
        "CLO.BSP",
        "CS2.BSP",
        "CS3.BSP",
        "CS5.BSP",
        "CS6.BSP",
        "CS8.BSP",
        "CSD.BSP",
        "CSE.BSP",
        "CSE_A.BSP",
        "CSE_B.BSP",
        //"DEFAULT.BSP",
        "DIN.BSP",
        "DIN_106P(AANDB)_205P(A).BSP",
        "DIN_106P(C)_207A-202P_205P(B).BSP",
        "DIN_106P_207A-202P_205P(B).BSP",
        "DIN_110A_112P(A).BSP",
        "DIN_112P(B)_102P_104P.BSP",
        "DIN_205P(C)_307A-312P_306P.BSP",
        "DIN_302A.BSP",
        "DIN_303P.BSP",
        "DU1.BSP",
        "DU2.BSP",
        "GRI.BSP",
        "HAL.BSP",
        "KIT.BSP",
        "LBY.BSP",
        "LER.BSP",
        "LHE.BSP",
        "LHI.BSP",
        "LHM.BSP",
        "LHM_A.BSP",
        "LHM_B.BSP",
        "LHM_C.BSP",
        "LMB.BSP",
        "MA1.BSP",
        "MA3.BSP",
        "MAG.BSP",
        "MCB.BSP",
        "MCF.BSP",
        "MOP.BSP",
        //"MOP_102P_WITHOUTVES01.BSP",
        //"MOP_102P_WITHVES01.BSP",
        //"MOP_110A.BSP",
        //"MOP_112P.BSP",
        "MS2.BSP",
        "MS3.BSP",
        "OFF.BSP",
        "PHO.BSP",
        "PL1_A.BSP",
        "PL1_B.BSP",
        "PL2_A.BSP",
        "PL2_B.BSP",
        "PL3.BSP",
        "PL4.BSP",
        "PL5.BSP",
        "PL6.BSP",
        "PL6_A_A.BSP",
        "PLO.BSP",
        "POU.BSP",
        "R21.BSP",
        "R23.BSP",
        "R25.BSP",
        "R27.BSP",
        "R29.BSP",
        "R31.BSP",
        "R33.BSP",
        "R33_N.BSP",
        "RC1_A.BSP",
        "RC1_B.BSP",
        "RC2.BSP",
        "RC3.BSP",
        "RC4_A.BSP",
        "RC4_B.BSP",
        "RL1.BSP",
        "RL2.BSP",
        "ROQ.BSP",
        "TE1.BSP",
        "TE3.BSP",
        "TE4A.BSP",
        "TE4TEST.BSP",
        "TE5.BSP",
        "TE6.BSP",
        "TE6_B4.BSP",
        "TR1.BSP",
        "TR2.BSP",
        "VG1.BSP",
        "VGR.BSP",
        "VGR_A_A.BSP",
        "WDB.BSP",
        "WOD.BSP",
    };
    for(auto& name : allBsp)
    {
        std::cout << "Loading " << name << " BSP..." << std::endl;
        gAssetManager.LoadBSP(name);
        std::cout << "Done loading " << name << " BSP." << std::endl;
    }
    */
}

void LocationManager::Update()
{
    // If a location change is pending, make it happen.
    if(!mChangeLocationTo.empty())
    {
        ChangeLocationInternal(mChangeLocationTo, mChangeLocationCallback);

        mChangeLocationTo.clear();
        mChangeLocationCallback = nullptr;
    }
}

bool LocationManager::IsValidLocation(const std::string& locationCode) const
{
    // All location codes are 3 characters exactly.
    if(locationCode.length() != 3) { return false; }

    bool isValid = mLocCodeShortToLocCodeLong.find(locationCode) != mLocCodeShortToLocCodeLong.end();
    if(!isValid)
    {
        gReportManager.Log("Error", "Error: '" + locationCode + "' is not a valid location name. Call DumpLocations() to see valid locations.");
    }
    return isValid;
}

void LocationManager::DumpLocations() const
{
    //TODO
}

void LocationManager::ChangeLocation(const std::string& location, std::function<void()> callback)
{
    // Record that we want to change location. This change request will be processed on next Update call.
    // We don't just change location right away b/c this function might be called at any time, but we want location change to happen at a known time in code.
    mChangeLocationTo = location;
    mChangeLocationCallback = callback;
}

void LocationManager::SetLocation(const std::string& location)
{
    if(!StringUtil::EqualsIgnoreCase(mLocation, location))
    {
        mLastLocation = mLocation;
        mLocation = location;
    }
}

std::string LocationManager::GetLocationDisplayName() const
{
    return GetLocationDisplayName(mLocation);
}

std::string LocationManager::GetLocationDisplayName(const std::string& location) const
{
    // Keys for timeblocks are in form "loc_r25".
    return gLocalizer.GetText("loc_" + location);
}

int LocationManager::GetLocationCountAcrossAllTimeblocks(const std::string& actorName, const std::string& location)
{
    return mActorLocationCounts[actorName + location];
}

int LocationManager::GetCurrentLocationCountForCurrentTimeblock(const std::string& actorName) const
{
    return GetLocationCount(actorName, mLocation, gGameProgress.GetTimeblock());
}

int LocationManager::GetLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location) const
{
    return GetLocationCount(actorName, location, gGameProgress.GetTimeblock());
}

int LocationManager::GetLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock) const
{
    return GetLocationCount(actorName, location, timeblock.ToString());
}

int LocationManager::GetLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock) const
{
    // Either return stored value, or 0 by default.
    auto it = mActorLocationTimeblockCounts.find(actorName + location + timeblock);
    if(it != mActorLocationTimeblockCounts.end())
    {
        return it->second;
    }
    return 0;
}

void LocationManager::IncCurrentLocationCountForCurrentTimeblock(const std::string& actorName)
{
    IncLocationCount(actorName, mLocation, gGameProgress.GetTimeblock());
}

void LocationManager::IncLocationCountForCurrentTimeblock(const std::string &actorName, const std::string &location)
{
    IncLocationCount(actorName, mLocation, gGameProgress.GetTimeblock());
}

void LocationManager::IncLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock)
{
    IncLocationCount(actorName, location, timeblock.ToString());
}

void LocationManager::IncLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock)
{
    // Increment global location count. Lowercase for consistency.
    ++mActorLocationCounts[actorName + location];

    // Increment timeblock-specific location count. Lowercase for consistency.
    ++mActorLocationTimeblockCounts[actorName + location + timeblock];
}

void LocationManager::SetLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location, int count)
{
    // Get current timeblock as string.
    std::string timeblock = gGameProgress.GetTimeblock().ToString();

    // Increment timeblock-specific location count. This version should NOT change the global one!
    mActorLocationTimeblockCounts[actorName + location + timeblock] = count;
}

void LocationManager::SetActorLocation(const std::string& actorName, const std::string& location)
{
    if(location.empty())
    {
        SetActorOffstage(actorName);
    }
    else
    {
        mActorLocations[actorName] = location;
    }
}

std::string LocationManager::GetActorLocation(const std::string& actorName) const
{
    auto it = mActorLocations.find(actorName);
    if(it != mActorLocations.end())
    {
        return it->second;
    }
    return "";
}

bool LocationManager::IsActorAtLocation(const std::string& actorName, const std::string& location) const
{
    auto it = mActorLocations.find(actorName);
    if(it != mActorLocations.end())
    {
        return StringUtil::EqualsIgnoreCase(it->second, location);
    }
    return false;
}

void LocationManager::SetActorOffstage(const std::string& actorName)
{
    auto it = mActorLocations.find(actorName);
    if(it != mActorLocations.end())
    {
        mActorLocations.erase(it);
    }
}

bool LocationManager::IsActorOffstage(const std::string& actorName) const
{
    auto it = mActorLocations.find(actorName);
    return it == mActorLocations.end();
}

void LocationManager::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mLocation));
    ps.Xfer(PERSIST_VAR(mLastLocation));

    ps.Xfer(PERSIST_VAR(mActorLocationCounts));
    ps.Xfer(PERSIST_VAR(mActorLocationTimeblockCounts));
    ps.Xfer(PERSIST_VAR(mActorLocations));
}

void LocationManager::ChangeLocationInternal(const std::string& location, std::function<void()> callback)
{
    // Show scene transitioner (except when transitioning from the Map screen).
    if(!StringUtil::EqualsIgnoreCase(mLocation, "map"))
    {
        gGK3UI.ShowSceneTransitioner();
    }

    // If the action bar is up, it should close on a scene change.
    gActionManager.HideActionBar(false);

    // Set new location.
    // This is important to do BEFORE checking for timeblock completion, as that logic looks for locations sometimes.
    bool sameLocation = StringUtil::EqualsIgnoreCase(mLocation, location);
    SetLocation(location);

    //HACK: Don't check timeblock completion if following someone on driving screen.
    //HACK: Fixes premature timeblock completion in 102P if last action performed is follow.
    if(gGK3UI.FollowingOnDrivingScreen())
    {
        // Change scene and done.
        gSceneManager.LoadScene(mChangeLocationTo, [callback](){
            gGK3UI.HideSceneTransitioner();
            if(callback != nullptr) { callback(); }
        });
        return;
    }

    // Check for timeblock completion.
    gSheepManager.Execute(gAssetManager.LoadSheep("Timeblocks.shp"), "CheckTimeblockComplete$", [this, location, callback, sameLocation](){

        // See whether a timeblock change is occurring.
        // If so, we should early out - the timeblock change logic handles any location and time change.
        if(gGameProgress.IsChangingTimeblock())
        {
            // Usually, GK3 doesn't allow location & last location to be the same place.
            // But at the beginning of Day 3 12PM, the initial position logic for Grace DEPENDS ON mLastLocation and mLocation being the same.
            // There may be a more elegant generalized way to handle this (e.g. on new timeblock, if switching ego, set last location to actor's location in mActorLocations).
            // But for now, this targeted HACK does the trick!
            if(gGameProgress.GetTimeblock() == Timeblock(2, 14) || gGameProgress.GetTimeblock() == Timeblock(3, 10))
            {
                mLastLocation = location;
            }

            // Early out and let timeblock code handle this.
            gGK3UI.HideSceneTransitioner();
            if(callback != nullptr) { callback(); }
            return;
        }

        // No need to change if we're already there.
        if(sameLocation)
        {
            gGK3UI.HideSceneTransitioner();
            if(callback != nullptr) { callback(); }
        }
        else
        {
            // Otherwise, we can move ahead with changing the scene.
            gSceneManager.LoadScene(location, [callback](){
                gGK3UI.HideSceneTransitioner();
                if(callback != nullptr) { callback(); }
            });
        }
    });
}
