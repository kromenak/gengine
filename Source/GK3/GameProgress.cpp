#include "GameProgress.h"

#include "GMath.h"
#include "IniParser.h"
#include "Localizer.h"
#include "Scene.h"
#include "Services.h"
#include "StatusOverlay.h"
#include "StringUtil.h"
#include "TextAsset.h"

TYPE_DEF_BASE(GameProgress);

GameProgress::GameProgress()
{
    // Parse valid score events (and score amount) int map of score events.
    TextAsset* textFile = Services::GetAssets()->LoadText("Scores.txt");
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        for(auto& line : section.lines)
        {
            for(auto& entry : line.entries)
            {
                int score = entry.GetValueAsInt();
                mScoreEvents[entry.key] = score;
            }
        }
    }
}

void GameProgress::SetScore(int score)
{
	mScore = Math::Clamp(score, 0, kMaxScore);
}

void GameProgress::IncreaseScore(int points)
{
	SetScore(mScore + points);
}

void GameProgress::ChangeScore(const std::string& scoreName)
{
    // Make sure it is a valid score name.
    auto validEventsIt = mScoreEvents.find(scoreName);
    if(validEventsIt == mScoreEvents.end())
    {
        Services::GetReports()->Log("Error", StringUtil::Format("Illegal score name (%s)", scoreName.c_str()));
        return;
    }

    // If we haven't already gotten this score event, we can now get it.
    auto achievedEventsIt = mScoreEventFlags.find(scoreName);
    if(achievedEventsIt == mScoreEventFlags.end())
    {
        // Flag that we've achieved this one.
        mScoreEventFlags[scoreName] = true;

        // Give the points.
        IncreaseScore(validEventsIt->second);

        // Refresh status overlay to show updated point count.
        Scene* scene = GEngine::Instance()->GetScene();
        if(scene != nullptr)
        {
            StatusOverlay* statusOverlay = scene->GetStatusOverlay();
            if(statusOverlay != nullptr)
            {
                statusOverlay->Refresh();
            }
        }
    }
}

void GameProgress::SetTimeblock(const Timeblock& timeblock)
{
	mLastTimeblock = mTimeblock;
	mTimeblock = timeblock;
	
	// Chat counts are reset on time block change.
	mChatCounts.clear();
}

std::string GameProgress::GetTimeblockDisplayName() const
{
    // Keys for timeblocks are in form "Day110A".
    return Services::Get<Localizer>()->GetText("Day" + mTimeblock.ToString());
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

int GameProgress::GetTopicCount(const std::string& noun, const std::string& topic) const
{
	// Key is noun+topic.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + topic;
	StringUtil::ToLower(key);
	
	// Find and return, or return default.
	auto it = mTopicCounts.find(key);
	if(it != mTopicCounts.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetTopicCount(const std::string& noun, const std::string& topic, int count)
{
	// Key is noun+topic.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + topic;
	StringUtil::ToLower(key);
	mTopicCounts[key] = count;
}

void GameProgress::IncTopicCount(const std::string& noun, const std::string& topic)
{
	// Key is noun+topic.
	// Make sure it's all lowercase, for consistency.
	std::string key = noun + topic;
	StringUtil::ToLower(key);
	++mTopicCounts[key];
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
