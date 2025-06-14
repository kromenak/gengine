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
#include <iostream>

class PersistState;

typedef char AMPM;

class Timeblock
{
public:
    static const AMPM AM = 0;
    static const AMPM PM = 1;

    Timeblock() = default;
    Timeblock(int day, int hour);
    Timeblock(int day, int hour, AMPM amOrPM);
    Timeblock(const std::string& str);

    std::string ToString() const;
    friend std::ostream& operator<< (std::ostream& o, const Timeblock& timeblock);

    bool operator==(const Timeblock& other) const;
    bool operator!=(const Timeblock& other) const;
    bool operator<(const Timeblock& other) const;
    bool operator>(const Timeblock& other) const;
    bool operator<=(const Timeblock& other) const;
    bool operator>=(const Timeblock& other) const;

    int GetDay() const { return mDay; }

    int GetHour24() const { return mHour; }
    int GetHour12() const;

    bool IsAM() const { return mHour < 12; }
    bool IsPM() const { return mHour >= 12; }

    AMPM GetAMPM() const { return IsAM() ? AM : PM; }
    char GetAMPMSuffix() const { return IsAM() ? 'A' : 'P'; }

    #if !defined(TESTS)
    void OnPersist(PersistState& ps);
    #endif

    static void ParseTimeblockRange(const std::string& name, Timeblock& start, Timeblock& end);

private:
    // The day; should be 1+.
    int mDay = 1;

    // The hour; should be 0-23.
    int mHour = 10;
};
