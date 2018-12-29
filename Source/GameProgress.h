//
// GameProgress.h
//
// Clark Kromenaker
//
// Tracks game progress/logic variables and the state of the player
// throughout the game. Keeps track of thinks like current/last location/time,
// flag states, game logic variable states, noun/verb counts, etc.
//
#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Type.h"

struct TimeBlock
{
	// The day; should be 1+.
	int day = 1;
	
	// The hour; should be 0-23.
	int hour = 10;
	
	// Depending on day/hour, returns something like "110A".
	std::string GetCode()
	{
		// Add A/P (for am/pm) on end, depending on the hour.
		// 0-11 is AM, 12-23 is PM.
		std::string ampm = (hour <= 11) ? "A" : "P";
		
		// The hour is 24-hour based. But in the code, it's am/pm.
		// If over 12, subtract 12.
		int ampmHour = hour > 12 ? hour - 12 : hour;
		
		// If hour is single digit, prepend a zero.
		std::string hourStr = std::to_string(ampmHour);
		if(ampmHour < 10)
		{
			hourStr = "0" + hourStr;
		}
		
		// Put it all together.
		return std::to_string(day) + hourStr + ampm;
	}
};

class GameProgress
{
	TYPE_DECL_BASE();
public:
	int GetMaxScore() const { return kMaxScore; }
	int GetScore() const { return mScore; }
	void SetScore(int score) { mScore = score; }
	void IncreaseScore(int points) { mScore += points; }
	
	std::string GetLocation() const { return mLocation; }
	std::string GetLastLocation() const { return mLastLocation; }
	void SetLocation(std::string location) { mLastLocation = mLocation; mLocation = location; }
	
	std::string GetTimeCode() const { return mTimeCode; }
	std::string GetLastTimeCode() const { return mLastTimeCode; }
	void SetTimeCode(std::string timeCode) { mLastTimeCode = mTimeCode; mTimeCode = timeCode; }
	
	bool GetFlag(std::string flagName) const;
	void SetFlag(std::string flagName);
	void ClearFlag(std::string flagName);
	
	int GetChatCount(std::string noun) const;
	void SetChatCount(std::string, int count);
	void IncChatCount(std::string noun);
	
	//TODO: Topic Counts
	
	int GetNounVerbCount(std::string noun, std::string verb) const;
	void SetNounVerbCount(std::string noun, std::string verb, int count);
	void IncNounVerbCount(std::string noun, std::string verb);
	
	int GetGameVariable(std::string varName) const;
	void SetGameVariable(std::string varName, int value);
	void IncGameVariable(std::string varName);
	
private:
	// Score tracking.
	const int kMaxScore = 999;
	int mScore = 0;
	
	// Current and last location.
	std::string mLocation = "R25";
	std::string mLastLocation;
	
	// Current and last time blocks.
	std::string mTimeCode = "110a";
	std::string mLastTimeCode;
	
	// General-use true/false flags for game logic.
	// Absence of a flag implies false; otherwise, true.
	std::unordered_set<std::string> mGameFlags;
	
	// Tracks the number of times the player has chatted with a noun.
	// TODO: Could this just be stored in noun/verbs?
	std::unordered_map<std::string, int> mChatCounts;
	
	// Maps noun/verb to a count value.
	// Tracks the number of times we've triggered a verb on a noun.
	std::unordered_map<std::string, std::unordered_map<std::string, int>> mNounVerbCounts;

	// Maps a variable name to an integer value.
	// For general game logic variables.
	std::unordered_map<std::string, int> mGameVariables;
};

