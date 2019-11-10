//
// Timeblock.h
//
// Clark Kromenaker
//
// In GK3, progress through the game is measured by what time block you are currently in.
// You can think of it kind of like a level. Once you finish all tasks in one time block, you move to the next.
//
// A timeblock is identified by a day and an hour. A timeblock can be represented in code using
// one of these Timeblock structs, or via a string. You can easily switch from one to the other.
//
#pragma once
#include <string>

typedef char AMPM;

class Timeblock
{
public:
	static const AMPM AM = 0;
	static const AMPM PM = 1;
	
	Timeblock(int day, int hour);
	Timeblock(int day, int hour, AMPM amOrPM);
	Timeblock(const std::string& str);
	
	std::string ToString() const;
	
	int GetDay() const { return mDay; }
	
	int GetHour24() const { return mHour; }
	int GetHour12() const;
	
	bool IsAM() const { return mHour < 12; }
	bool IsPM() const { return mHour >= 12; }
	
	AMPM GetAMPM() const { return IsAM() ? AM : PM; }
	char GetAMPMSuffix() const { return IsAM() ? 'A' : 'P'; }
	
private:
	// The day; should be 1+.
	int mDay = 1;
	
	// The hour; should be 0-23.
	int mHour = 10;
};
