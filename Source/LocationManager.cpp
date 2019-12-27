//
// LocationManager.cpp
//
// Clark Kromenaker
//
#include "LocationManager.h"

#include "GameProgress.h"
#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"
#include "Timeblock.h"

TYPE_DEF_BASE(LocationManager);

LocationManager::LocationManager()
{
	unsigned int bufferSize = 0;
	char* buffer = Services::GetAssets()->LoadRaw("Locations.txt", bufferSize);
	
	// Parse as INI file.
	IniParser parser(buffer, bufferSize);
	parser.ParseAll();
	
	IniSection locations = parser.GetSection("LOCATIONS");
	for(auto& line : locations.lines)
	{
		IniKeyValue& entry = line.entries.front();
		mLocCodeShortToLocCodeLong[entry.key] = entry.value;
	}
	
	delete[] buffer;
}

bool LocationManager::IsValidLocation(const std::string& locationCode) const
{
	// All location codes are 3 characters exactly.
	if(locationCode.length() != 3) { return false; }
	
	std::string key = StringUtil::ToLowerCopy(locationCode);
	return mLocCodeShortToLocCodeLong.find(key) != mLocCodeShortToLocCodeLong.end();
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
