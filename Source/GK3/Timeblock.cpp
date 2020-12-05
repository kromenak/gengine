//
// Timeblock.cpp
//
// Clark Kromenaker
//
#include "Timeblock.h"

Timeblock::Timeblock(int day, int hour) : mDay(day), mHour(hour)
{
	if(mDay < 1) { mDay = 1; }
	if(mHour < 0) { mHour = 0; }
	else if(mHour > 23) { mHour = 23; }
}

Timeblock::Timeblock(int day, int hour, AMPM amOrPM) : mDay(day)
{
	// Day is easy.
	if(mDay < 1) { mDay = 1; }
	
	// Make sure hour is between 1-12
	if(hour < 1) { hour = 1; }
	else if(hour > 12) { hour = 12; }
	
	// Convert to 24-hour.
	bool am = (amOrPM == AM);
	if(am && hour == 12)
	{
		mHour = 0;
	}
	else if(!am && hour < 12)
	{
		mHour = hour + 12;
	}
	else
	{
		mHour = hour;
	}
}

Timeblock::Timeblock(const std::string& str)
{
	// We'll do our best to turn this string into a day/hour.
	// We're going to assume the form of "110A" or "205P".
	if(str.size() < 4) { return; }
	
	// First digit must be the day.
	if(!std::isdigit(str[0])) { return; }
	mDay = std::stoi(std::string(1, str[0]));
	
	// Next two digits must be the hour, 12-hour based, with A/P afterwards for AM/PM.
	if(!std::isdigit(str[1]) || !std::isdigit(str[2])) { return; }
	std::string hourStr = str.substr(1, 2);
	mHour = std::stoi(hourStr);
	
	// Determine whether this is AM or PM.
	bool isPM = (str[3] == 'p' || str[3] == 'P');
	
	// If PM, and hour is less than 12, add 12.
	// Ex: 1PM becomes 13, 11PM becomes 23.
	if(isPM && mHour < 12)
	{
		mHour += 12;
	}
	
	// If AM, and hour is 12, then it's ACTUALLY 0!
	// Ex: 12AM becomes 0, 1AM stays 1, 11AM stays 11.
	if(!isPM && mHour == 12)
	{
		mHour = 0;
	}
}

std::string Timeblock::ToString() const
{
	// Add A/P (for am/pm) on end, depending on the hour.
	// 0-11 is AM, 12-23 is PM.
	std::string ampm = (mHour <= 11) ? "A" : "P";
	
	// The hour is 24-hour based. But in the string, it's am/pm.
	// If over 12, subtract 12.
	int ampmHour = mHour > 12 ? mHour - 12 : mHour;
	if(ampmHour < 1)
	{
		ampmHour = 12;
	}
	
	// If hour is single digit, prepend a zero.
	std::string hourStr = std::to_string(ampmHour);
	if(ampmHour < 10)
	{
		hourStr = "0" + hourStr;
	}
	
	// Put it all together.
	return std::to_string(mDay) + hourStr + ampm;
}

std::ostream& operator<<(std::ostream& o, const Timeblock& timeblock)
{
	return o << timeblock.ToString();
}

bool Timeblock::operator==(const Timeblock& other) const
{
	return mDay == other.mDay && mHour == other.mHour;
}

bool Timeblock::operator!=(const Timeblock& other) const
{
	return !(*this == other);
}

int Timeblock::GetHour12() const
{
	if(mHour == 0) { return 12; }
	return mHour > 12 ? mHour - 12 : mHour;
}
