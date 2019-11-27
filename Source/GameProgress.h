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

#include "Timeblock.h"
#include "Type.h"

class GameProgress
{
	TYPE_DECL_BASE();
public:
	int GetMaxScore() const { return kMaxScore; }
	int GetScore() const { return mScore; }
	void SetScore(int score);
	void IncreaseScore(int points);
	
	const Timeblock& GetTimeblock() const { return mTimeblock; }
	const Timeblock& GetLastTimeblock() const { return mLastTimeblock; }
	void SetTimeblock(const Timeblock& timeblock);
	
	bool GetFlag(const std::string& flagName) const;
	void SetFlag(const std::string& flagName);
	void ClearFlag(const std::string& flagName);
	
	int GetGameVariable(const std::string& varName) const;
	void SetGameVariable(const std::string& varName, int value);
	void IncGameVariable(const std::string& varName);
	
	//TODO: Chat counts should be reset when the timeblock changes!
	int GetChatCount(const std::string& noun) const;
	void SetChatCount(const std::string&, int count);
	void IncChatCount(const std::string& noun);
	
	//TODO: Topic Counts
	
	int GetNounVerbCount(const std::string& noun, const std::string& verb) const;
	void SetNounVerbCount(const std::string& noun, const std::string& verb, int count);
	void IncNounVerbCount(const std::string& noun, const std::string& verb);
	
private:
	// Score tracking.
	const int kMaxScore = 999;
	int mScore = 0;
	
	// Current and last time blocks.
	Timeblock mTimeblock;
	Timeblock mLastTimeblock;
	
	// General-use true/false flags for game logic.
	// Absence of a flag implies false; otherwise, true.
	std::unordered_set<std::string> mGameFlags;
	
	// Tracks the number of times the player has chatted with a noun.
	// TODO: Could this just be stored in noun/verbs?
	std::unordered_map<std::string, int> mChatCounts;
	
	// Maps noun/verb to a count value.
	// Tracks the number of times we've triggered a verb on a noun.
	std::unordered_map<std::string, int> mNounVerbCounts;

	// Maps a variable name to an integer value.
	// For general game logic variables.
	std::unordered_map<std::string, int> mGameVariables;
};

