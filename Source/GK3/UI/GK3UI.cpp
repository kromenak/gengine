#include "GK3UI.h"

#include "CaptionsOverlay.h"
#include "DrivingScreen.h"
#include "FinishedScreen.h"
#include "SceneTransitioner.h"
#include "Sidney.h"
#include "TimeblockScreen.h"
#include "TitleScreen.h"

GK3UI gGK3UI;

namespace
{
    // NOTE: Putting this here to avoid name clash with "FinishedScreen" Sheep API call.
    FinishedScreen* mFinishedScreen = nullptr;
}

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

void GK3UI::ShowFinishedScreen()
{
    if(mFinishedScreen == nullptr)
    {
        mFinishedScreen = new FinishedScreen();
        mFinishedScreen->SetIsDestroyOnLoad(false);
    }
    mFinishedScreen->Show();
}

void GK3UI::ShowSceneTransitioner()
{
    if(mSceneTransitioner == nullptr)
    {
        mSceneTransitioner = new SceneTransitioner();
        mSceneTransitioner->SetIsDestroyOnLoad(false);
    }
    mSceneTransitioner->Show();
}

void GK3UI::HideSceneTransitioner()
{
    if(mSceneTransitioner != nullptr)
    {
        mSceneTransitioner->Hide();
    }
}

void GK3UI::AddCaption(const std::string& caption, const std::string& speaker)
{
    if(mCaptionsOverlay == nullptr)
    {
        mCaptionsOverlay = new CaptionsOverlay();
        mCaptionsOverlay->SetIsDestroyOnLoad(false);
    }
    mCaptionsOverlay->AddCaption(caption, speaker);
}

void GK3UI::FinishCaption(float delay)
{
    if(mCaptionsOverlay != nullptr)
    {
        mCaptionsOverlay->AdvanceCaption(delay);
    }
}

void GK3UI::HideAllCaptions()
{
    if(mCaptionsOverlay != nullptr)
    {
        mCaptionsOverlay->HideAll();
    }
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
        mDrivingScreen->Show(DrivingScreen::FollowMode::LadyHoward);
    }
    else if(followState == 5)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::PrinceJamesMen);
    }
    else if(followState == 6)
    {
        mDrivingScreen->Show(DrivingScreen::FollowMode::Estelle);
    }
    else
    {
        mDrivingScreen->Show();
    }
}

bool GK3UI::FollowingOnDrivingScreen()
{
    if(mDrivingScreen != nullptr)
    {
        return mDrivingScreen->FollowingSomeone();
    }
    return false;
}

Sidney* GK3UI::GetSidney()
{
    if(mSidney == nullptr)
    {
        mSidney = new Sidney();
        mSidney->SetIsDestroyOnLoad(false);
    }
    return mSidney;
}

void GK3UI::ShowSidney()
{
    GetSidney()->Show();
}