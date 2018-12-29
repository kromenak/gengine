//
// GameProgress.cpp
//
// Clark Kromenaker
//
#include "GameProgress.h"

TYPE_DEF_BASE(GameProgress);

bool GameProgress::GetFlag(std::string flagName) const
{
	// If the flag exists, it implies a "true" value.
	// Absence of flag implies "false" value.
	auto it = mGameFlags.find(flagName);
	return it != mGameFlags.end();
}

void GameProgress::SetFlag(std::string flagName)
{
	// Doesn't matter whether we are setting an already set flag.
	mGameFlags.insert(flagName);
}

void GameProgress::ClearFlag(std::string flagName)
{
	// Erase the flag from the container to "clear" it.
	auto it = mGameFlags.find(flagName);
	if(it != mGameFlags.end())
	{
		mGameFlags.erase(it);
	}
}

int GameProgress::GetChatCount(std::string noun) const
{
	auto it = mChatCounts.find(noun);
	if(it != mChatCounts.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetChatCount(std::string noun, int count)
{
	mChatCounts[noun] = count;
}

void GameProgress::IncChatCount(std::string noun)
{
	mChatCounts[noun]++;
}

int GameProgress::GetNounVerbCount(std::string noun, std::string verb) const
{
	auto it = mNounVerbCounts.find(noun);
	if(it != mNounVerbCounts.end())
	{
		auto it2 = it->second.find(verb);
		if(it2 != it->second.end())
		{
			return it2->second;
		}
	}
	return 0;
}

void GameProgress::SetNounVerbCount(std::string noun, std::string verb, int count)
{
	mNounVerbCounts[noun][verb] = count;
}

void GameProgress::IncNounVerbCount(std::string noun, std::string verb)
{
	mNounVerbCounts[noun][verb]++;
}

int GameProgress::GetGameVariable(std::string varName) const
{
	auto it = mGameVariables.find(varName);
	if(it != mGameVariables.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetGameVariable(std::string varName, int value)
{
	mGameVariables[varName] = value;
}

void GameProgress::IncGameVariable(std::string varName)
{
	mGameVariables[varName]++;
}
