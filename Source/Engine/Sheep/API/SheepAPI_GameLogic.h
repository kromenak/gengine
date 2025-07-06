//
// Clark Kromenaker
//
// Sheep SysFyncs related to building gameplay.
// This includes tracking what actions were performed, scoring, etc.
//
#pragma once
#include "SheepSysFunc.h"

// SCORING
int GetScore();
shpvoid IncreaseScore(int value);
shpvoid SetScore(int score); // DEV
shpvoid ChangeScore(const std::string& scoreValue);

// FLAGS
int GetFlag(const std::string& flagName);
int GetFlagInt(int flagEnum);
shpvoid SetFlag(const std::string& flagName);
shpvoid ClearFlag(const std::string& flagName);
shpvoid DumpFlags(); // DEV

// VARIABLES
int GetGameVariableInt(const std::string& varName);
shpvoid IncGameVariableInt(const std::string& varName);
shpvoid SetGameVariableInt(const std::string& varName, int value);

// ACTION TRACKING
int GetNounVerbCount(const std::string& noun, const std::string& verb);
int GetNounVerbCountInt(int nounEnum, int verbEnum);
shpvoid IncNounVerbCount(const std::string& noun, const std::string& verb);
shpvoid IncNounVerbCountBoth(const std::string& noun, const std::string& verb);
shpvoid SetNounVerbCount(const std::string& noun, const std::string& verb, int count);
shpvoid SetNounVerbCountBoth(const std::string& noun, const std::string& verb, int count);
shpvoid TriggerNounVerb(const std::string& noun, const std::string& verb); // DEV

int GetTopicCount(const std::string& noun, const std::string& verb);
int GetTopicCountInt(int nounEnum, int verbEnum);
int HasTopicsLeft(const std::string& noun);
shpvoid SetTopicCount(std::string noun, std::string verb, int count); // DEV

int GetChatCount(const std::string& noun);
int GetChatCountInt(int nounEnum);
shpvoid SetChatCount(std::string noun, int count); // DEV

shpvoid FullReset(); // DEV
shpvoid ResetGameData(); // DEV

shpvoid SetVerbModal(int modalState);
shpvoid StartVerbCancel();
shpvoid StopVerbCancel();

// GAME BEHAVIOR
shpvoid SetGameTimer(const std::string& noun, const std::string& verb, int milliseconds);
shpvoid SetTimerMs(int milliseconds); // WAIT
shpvoid SetTimerSeconds(float seconds); // WAIT

float GetTimeMultiplier();
shpvoid SetTimeMultiplier(float multiplier);

float GetRandomFloat(float lower, float upper);
int GetRandomInt(int lower, int upper);
