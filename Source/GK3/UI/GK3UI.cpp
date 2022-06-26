#include "GK3UI.h"

#include "DrivingScreen.h"
#include "TimeblockScreen.h"
#include "TitleScreen.h"

GK3UI gGK3UI;

void GK3UI::ShowTitleScreen()
{
    if(mTitleScreen == nullptr)
    {
        mTitleScreen = new TitleScreen();
        mTitleScreen->SetIsDestroyOnLoad(false);
    }
    mTitleScreen->Show();
}

void GK3UI::ShowTimeblockScreen(const Timeblock& timeblock, float timer, std::function<void()> callback)
{
    if(mTimeblockScreen == nullptr)
    {
        mTimeblockScreen = new TimeblockScreen();
        mTimeblockScreen->SetIsDestroyOnLoad(false);
    }
    mTimeblockScreen->Show(timeblock, timer, callback);
}

void GK3UI::ShowDrivingScreen(int followState)
{
    if(mDrivingScreen == nullptr)
    {
        mDrivingScreen = new DrivingScreen();
        mDrivingScreen->SetIsDestroyOnLoad(false);
    }
    mDrivingScreen->Show();
}