#include "Timeblock.h"

#include <climits>

#include "StringUtil.h"

#if !defined(TESTS)
#include "PersistState.h"
#endif

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

bool Timeblock::operator<(const Timeblock& other) const
{
    // Definitely "less than" if my day is before other's day.
    if(mDay < other.mDay) { return true; }

    // Definitely NOT "less than" if my day is after other's day.
    if(mDay > other.mDay) { return false; }

    // Same day - check hour.
    return mHour < other.mHour;
}

bool Timeblock::operator>(const Timeblock& other) const
{
    // Must be greater if neither less than nor equal to...
    bool lessThanOrEqualTo = (*this < other) || (*this == other);
    return !lessThanOrEqualTo;
}

bool Timeblock::operator<=(const Timeblock& other) const
{
    return (*this < other) || (*this == other);
}

bool Timeblock::operator>=(const Timeblock& other) const
{
    return (*this > other) || (*this == other);
}

int Timeblock::GetHour12() const
{
    if(mHour == 0) { return 12; }
    return mHour > 12 ? mHour - 12 : mHour;
}

#if !defined(TESTS)
void Timeblock::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mDay));
    ps.Xfer(PERSIST_VAR(mHour));
}
#endif

/*static*/ void Timeblock::ParseTimeblockRange(const std::string& name, Timeblock& start, Timeblock& end)
{
    // Many assets in GK3 specify which timeblocks they apply to in the asset name. The HAL location demonstrates all variations:
    // HAL_ALL: applies to all timeblocks
    // HAL_1ALL: applies to all day 1 timeblocks
    // HAL_12ALL: applies to all day 1 & day 2 timeblocks
    // HAL110A04P: applies to timeblocks between 110A to 104P
    // HAL106P: applies only to 106P timeblock

    // Sometimes, we need to know if an asset is meant to be used in a particular timeblock. This function figures that out!

    // Sanitize the asset name a bit. All lowercase, remove any extension.
    std::string lowerName = StringUtil::ToLowerCopy(name);
    std::size_t dotPos = lowerName.find('.');
    if(dotPos != std::string::npos)
    {
        lowerName.erase(dotPos);
    }

    // There is some minimum length for these strings to be valid.
    // The shortest is sometuing like "HAL106P", which is at least 7 characters.
    if(lowerName.length() < 7)
    {
        return;
    }

    // The first three letters are always a location code. Skip them.
    // Also, we skip one more if there's an optional underscore.
    std::size_t curIndex = lowerName[3] == '_' ? 4 : 3;

    // See if "all" is in the name.
    // If so, it indicates that the actions are used for all timeblocks on one or more days.
    std::size_t allPos = lowerName.find("all", curIndex);
    if(allPos != std::string::npos)
    {
        // If "all" is at the current index, it means there's no day constraint - just ALWAYS load this one!
        if(allPos == curIndex)
        {
            start = Timeblock(1, 0);
            end = Timeblock(9, 23);
        }
        else // "all" is later in the string, meaning this asset is likely used in multiple days
        {
            // Try to parse the digits into days.
            int minDay = INT_MAX;
            int maxDay = INT_MIN;
            for(std::size_t i = curIndex; i < allPos; ++i)
            {
                if(std::isdigit(lowerName[i]))
                {
                    int day = lowerName[i] - '0';
                    if(day < minDay)
                    {
                        minDay = day;
                    }
                    if(day > maxDay)
                    {
                        maxDay = day;
                    }
                }
            }

            // If there were no digits found, we're parsing garbage and we should get outta here.
            if(minDay == INT_MAX || maxDay == INT_MIN)
            {
                return;
            }

            start = Timeblock(minDay, 0);
            end = Timeblock(maxDay, 23);
        }
    }
    else // "all" is not in the name, so this is either a timeblock range or a specific timeblock
    {
        // The next three letters should be a timeblock string.
        if(curIndex + 3 < lowerName.length())
        {
            std::string timeblockStr = lowerName.substr(curIndex, 4);
            start = Timeblock(timeblockStr);

            curIndex += 4;
            std::string endTimeblockStr = lowerName.substr(curIndex);
            if(endTimeblockStr.length() == 3)
            {
                endTimeblockStr = timeblockStr[0] + endTimeblockStr;
                end = Timeblock(endTimeblockStr);
            }
            else
            {
                end = start;
            }
        }
    }
}