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
    // Create driving screen if needed.
    if(mDrivingScreen == nullptr)
    {
        mDrivingScreen = new DrivingScreen();
        mDrivingScreen->SetIsDestroyOnLoad(false);
    }

    // Show driving screen in correct mode.
    if(followState == 1)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::Buthane);
    }
    else if(followState == 2)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::Wilkes);
    }
    // 3 is supposed to be for following Mosely during 102P timeblock. However, this appears to not be in the final game?
    else if(followState == 4)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::LadyHowardEstelle1);
    }
    else if(followState == 5)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::PrinceJamesMen);
    }
    else if(followState == 6)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::LadyHowardEstelle2);
    }
    else
    {
        mDrivingScreen->Show();
    }
}