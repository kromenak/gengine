//
// Clark Kromenaker
//
// A helper class for showing/hiding various GK3 UIs.
//
#pragma once
#include <functional>

class DrivingScreen;
class Timeblock;
class TimeblockScreen;
class TitleScreen;

class GK3UI
{
public:
    void ShowTitleScreen();
    void ShowTimeblockScreen(const Timeblock& timeblock, float timer, std::function<void()> callback);
    void ShowDrivingScreen(int followState = 0);

private:
    TitleScreen* mTitleScreen = nullptr;
    TimeblockScreen* mTimeblockScreen = nullptr;
    DrivingScreen* mDrivingScreen = nullptr;
};

extern GK3UI gGK3UI;