//
// GameProgress.cpp
//
// Clark Kromenaker
//
#include "GameProgress.h"

#include "Math.h"
#include "StringUtil.h"

TYPE_DEF_BASE(GameProgress);

void GameProgress::SetScore(int score)
{
	mScore = Math::Clamp(score, 0, kMaxScore);
}

void GameProgress::IncreaseScore(int points)
{
	SetScore(mScore + points);
}

void GameProgress::SetLocation(const std::string& location)
{
	mLastLocation = mLocation;
	mLocation = location;
}

void GameProgress::SetTimeCode(const std::string& timeCode)
{
	mLastTimeCode = mTimeCode;
	mTimeCode = timeCode;
}

bool GameProgress::GetFlag(const std::string& flagName) const
{
	// If the flag exists, it implies a "true" value.
	// Absence of flag implies "false" value.
	auto it = mGameFlags.find(flagName);
	return it != mGameFlags.end();
}

void GameProgress::SetFlag(const std::string& flagName)
{
	// Doesn't matter whether we are setting an already set flag.
	mGameFlags.insert(flagName);
}

void GameProgress::ClearFlag(const std::string& flagName)
{
	// Erase the flag from the container to "clear" it.
	auto it = mGameFlags.find(flagName);
	if(it != mGameFlags.end())
	{
		mGameFlags.erase(it);
	}
}

int GameProgress::GetLifetimeLocationCount(const std::string& actorName, const std::string& location)
{
	std::string key = actorName + location;
	StringUtil::ToLower(key);
	return mActorLocationCounts[key];
}

int GameProgress::GetLocationCount(const std::string& actorName) const
{
	return GetLocationCount(actorName, mLocation, mTimeCode);
}

int GameProgress::GetLocationCount(const std::string& actorName, const std::string& location) const
{
	return GetLocationCount(actorName, location, mTimeCode);
}

int GameProgress::GetLocationCount(const std::string& location, const std::string& timeblock, const std::string& actorName) const
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

void GameProgress::IncLocationCount(const std::string& actorName)
{
	IncLocationCount(actorName, mLocation, mTimeCode);
}

void GameProgress::IncLocationCount(const std::string &actorName, const std::string &location)
{
	IncLocationCount(actorName, mLocation, mTimeCode);
}

void GameProgress::IncLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock)
{
	// Generate a key from the various bits.
	// Make sure it's all lowercase, for consistency.
	std::string locationKey = actorName + location;
	std::string locationTimeblockKey = locationKey + timeblock;
	StringUtil::ToLower(locationKey);
	StringUtil::ToLower(locationTimeblockKey);
	
	++mActorLocationCounts[locationKey];
	++mActorLocationTimeblockCounts[locationTimeblockKey];
}

int GameProgress::GetChatCount(const std::string& noun) const
{
	auto it = mChatCounts.find(StringUtil::ToLowerCopy(noun));
	if(it != mChatCounts.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetChatCount(const std::string& noun, int count)
{
	mChatCounts[StringUtil::ToLowerCopy(noun)] = count;
}

void GameProgress::IncChatCount(const std::string& noun)
{
	++mChatCounts[StringUtil::ToLowerCopy(noun)];
}

int GameProgress::GetNounVerbCount(const std::string& noun, const std::string& verb) const
{
	// Key is noun+verb.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + verb;
	StringUtil::ToLower(key);
	
	// Find and return, or return default.
	auto it = mNounVerbCounts.find(key);
	if(it != mNounVerbCounts.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetNounVerbCount(const std::string& noun, const std::string& verb, int count)
{
	// Key is noun+verb.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + verb;
	StringUtil::ToLower(key);
	
	mNounVerbCounts[key] = count;
}

void GameProgress::IncNounVerbCount(const std::string& noun, const std::string& verb)
{
	// Key is noun+verb.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + verb;
	StringUtil::ToLower(key);
	
	++mNounVerbCounts[key];
}

int GameProgress::GetGameVariable(const std::string& varName) const
{
	auto it = mGameVariables.find(StringUtil::ToLowerCopy(varName));
	if(it != mGameVariables.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetGameVariable(const std::string& varName, int value)
{
	mGameVariables[StringUtil::ToLowerCopy(varName)] = value;
}

void GameProgress::IncGameVariable(const std::string& varName)
{
	++mGameVariables[StringUtil::ToLowerCopy(varName)];
}
