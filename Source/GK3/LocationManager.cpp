//
// LocationManager.cpp
//
// Clark Kromenaker
//
#include "LocationManager.h"

#include "GameProgress.h"
#include "IniParser.h"
#include "Localizer.h"
#include "Services.h"
#include "StringUtil.h"
#include "Timeblock.h"

TYPE_DEF_BASE(LocationManager);

LocationManager::LocationManager()
{
	TextAsset* textFile = Services::GetAssets()->LoadText("Locations.txt");
	
	// Parse as INI file.
	IniParser parser(textFile->GetText(), textFile->GetTextLength());
	parser.ParseAll();
	
	IniSection locations = parser.GetSection("LOCATIONS");
	for(auto& line : locations.lines)
	{
		IniKeyValue& entry = line.entries.front();
		mLocCodeShortToLocCodeLong[entry.key] = entry.value;
    }
    
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
        Services::GetAssets()->LoadBSP(name);
        std::cout << "Done loading " << name << " BSP." << std::endl;
    }
    */
}

bool LocationManager::IsValidLocation(const std::string& locationCode) const
{
	// All location codes are 3 characters exactly.
	if(locationCode.length() != 3) { return false; }
	
	std::string key = StringUtil::ToLowerCopy(locationCode);
	bool isValid = mLocCodeShortToLocCodeLong.find(key) != mLocCodeShortToLocCodeLong.end();
	if(!isValid)
	{
		Services::GetReports()->Log("Error", "Error: '" + locationCode + "' is not a valid location name. Call DumpLocations() to see valid locations.");
	}
	return isValid;
}

void LocationManager::DumpLocations() const
{
	//TODO
}

void LocationManager::SetLocation(const std::string& location)
{
	mLastLocation = mLocation;
	mLocation = location;
}

std::string LocationManager::GetLocationDisplayName() const
{
    return GetLocationDisplayName(mLocation);
}

std::string LocationManager::GetLocationDisplayName(const std::string& location) const
{
    // Keys for timeblocks are in form "loc_r25".
    return Services::Get<Localizer>()->GetText("loc_" + location);
}

int LocationManager::GetLocationCountAcrossAllTimeblocks(const std::string& actorName, const std::string& location)
{
	std::string key = actorName + location;
	StringUtil::ToLower(key);
	return mActorLocationCounts[key];
}

int LocationManager::GetCurrentLocationCountForCurrentTimeblock(const std::string& actorName) const
{
	return GetLocationCount(actorName, mLocation, Services::Get<GameProgress>()->GetTimeblock());
}

int LocationManager::GetLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location) const
{
	return GetLocationCount(actorName, location, Services::Get<GameProgress>()->GetTimeblock());
}

int LocationManager::GetLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock) const
{
	return GetLocationCount(actorName, location, timeblock.ToString());
}

int LocationManager::GetLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock) const
{
	// Generate a key from the various bits.
	// Make sure it's all lowercase, for consistency.
	std::string key = actorName + location + timeblock;
	StringUtil::ToLower(key);
	
	// Either return stored value, or 0 by default.
	auto it = mActorLocationTimeblockCounts.find(key);
	if(it != mActorLocationTimeblockCounts.end())
	{
		return it->second;
	}
	return 0;
}

void LocationManager::IncCurrentLocationCountForCurrentTimeblock(const std::string& actorName)
{
	IncLocationCount(actorName, mLocation, Services::Get<GameProgress>()->GetTimeblock());
}

void LocationManager::IncLocationCountForCurrentTimeblock(const std::string &actorName, const std::string &location)
{
	IncLocationCount(actorName, mLocation, Services::Get<GameProgress>()->GetTimeblock());
}

void LocationManager::IncLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock)
{
	IncLocationCount(actorName, location, timeblock.ToString());
}

void LocationManager::IncLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock)
{
	// Increment global location count. Lowercase for consistency.
	std::string locationKey = actorName + location;
	StringUtil::ToLower(locationKey);
	++mActorLocationCounts[locationKey];
	
	// Increment timeblock-specific location count. Lowercase for consistency.
	std::string locationTimeblockKey = locationKey + timeblock;
	StringUtil::ToLower(locationTimeblockKey);
	++mActorLocationTimeblockCounts[locationTimeblockKey];
}

void LocationManager::SetLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location, int count)
{
	// Get current timeblock as string.
	std::string timeblock = Services::Get<GameProgress>()->GetTimeblock().ToString();

	// Increment timeblock-specific location count. This version should NOT change the global one!
	std::string key = actorName + location + timeblock;
	StringUtil::ToLower(key);
	mActorLocationTimeblockCounts[key] = count;
}

void LocationManager::SetActorLocation(const std::string& actorName, const std::string& location)
{
	if(location.empty())
	{
		SetActorOffstage(actorName);
	}
	else
	{
		mActorLocations[StringUtil::ToLowerCopy(actorName)] = StringUtil::ToLowerCopy(location);
	}
}

std::string LocationManager::GetActorLocation(const std::string& actorName) const
{
	auto it = mActorLocations.find(StringUtil::ToLowerCopy(actorName));
	if(it != mActorLocations.end())
	{
		return it->second;
	}
	return "";
}

void LocationManager::SetActorOffstage(const std::string& actorName)
{
	auto it = mActorLocations.find(StringUtil::ToLowerCopy(actorName));
	if(it != mActorLocations.end())
	{
		mActorLocations.erase(it);
	}
}

bool LocationManager::IsActorOffstage(const std::string& actorName) const
{
	auto it = mActorLocations.find(StringUtil::ToLowerCopy(actorName));
	return it == mActorLocations.end();
}
