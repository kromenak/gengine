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
	
	std::string GetLocation() const { return mLocation; }
	std::string GetLastLocation() const { return mLastLocation; }
	void SetLocation(const std::string& location);
	
	const Timeblock& GetTimeblock() const { return mTimeblock; }
	const Timeblock& GetLastTimeblock() const { return mLastTimeblock; }
	void SetTimeblock(const Timeblock& timeblock);
	
	bool GetFlag(const std::string& flagName) const;
	void SetFlag(const std::string& flagName);
	void ClearFlag(const std::string& flagName);
	
	int GetGameVariable(const std::string& varName) const;
	void SetGameVariable(const std::string& varName, int value);
	void IncGameVariable(const std::string& varName);
	
	int GetLocationCountAcrossAllTimeblocks(const std::string& actorName, const std::string& location);
	int GetCurrentLocationCountForCurrentTimeblock(const std::string& actorName) const;
	int GetLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location) const;
	int GetLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock) const;
	int GetLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock) const;
	
	void IncCurrentLocationCountForCurrentTimeblock(const std::string& actorName);
	void IncLocationCountForCurrentTimeblock(const std::string& actorName, const std::string& location);
	void IncLocationCount(const std::string& actorName, const std::string& location, const Timeblock& timeblock);
	void IncLocationCount(const std::string& actorName, const std::string& location, const std::string& timeblock);
	
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
	
	// Current and last location.
	std::string mLocation = "non";
	std::string mLastLocation;
	
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
	
	// Location counts for actors. We track lifetime times an actor visits a location AND per-timeblock counts.
	// Key is actorName+location (e.g. gabrielr25) or actorName+locationId+timeblockCode (e.g. gabrielr25110a).
	// Value is number of times the actor has been at that location during that timeblock.
	std::unordered_map<std::string, int> mActorLocationCounts;
	std::unordered_map<std::string, int> mActorLocationTimeblockCounts;
};

