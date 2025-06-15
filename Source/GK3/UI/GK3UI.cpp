#include "GK3UI.h"

#include "ActionManager.h"
#include "BinocsOverlay.h"
#include "CaptionsOverlay.h"
#include "DeathScreen.h"
#include "DrivingScreen.h"
#include "FingerprintScreen.h"
#include "FinishedScreen.h"
#include "GPSOverlay.h"
#include "InputManager.h"
#include "ProgressBar.h"
#include "PauseScreen.h"
#include "QuitPopup.h"
#include "SaveLoadScreen.h"
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

void GK3UI::HideTitleScreen()
{
    if(mTitleScreen != nullptr)
    {
        mTitleScreen->Hide();
    }
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

void GK3UI::ShowDeathScreen()
{
    if(mDeathScreen == nullptr)
    {
        mDeathScreen = new DeathScreen();
        mDeathScreen->SetIsDestroyOnLoad(false);
    }
    mDeathScreen->Show();
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

void GK3UI::ShowSaveScreen()
{
    if(mSaveLoadScreen == nullptr)
    {
        mSaveLoadScreen = new SaveLoadScreen();
        mSaveLoadScreen->SetIsDestroyOnLoad(false);
    }
    mSaveLoadScreen->ShowSave();
}

void GK3UI::ShowLoadScreen()
{
    if(mSaveLoadScreen == nullptr)
    {
        mSaveLoadScreen = new SaveLoadScreen();
        mSaveLoadScreen->SetIsDestroyOnLoad(false);
    }
    mSaveLoadScreen->ShowLoad();
}

void GK3UI::ShowGenericProgressBar()
{
    if(mProgressBar == nullptr)
    {
        mProgressBar = new ProgressBar();
        mProgressBar->SetIsDestroyOnLoad(false);
    }
    mProgressBar->Show(ProgressBar::Type::Generic);
}

void GK3UI::ShowSaveProgressBar()
{
    if(mProgressBar == nullptr)
    {
        mProgressBar = new ProgressBar();
        mProgressBar->SetIsDestroyOnLoad(false);
    }
    mProgressBar->Show(ProgressBar::Type::Save);
}

void GK3UI::ShowLoadProgressBar()
{
    if(mProgressBar == nullptr)
    {
        mProgressBar = new ProgressBar();
        mProgressBar->SetIsDestroyOnLoad(false);
    }
    mProgressBar->Show(ProgressBar::Type::Load);
}

void GK3UI::SetProgressBarProgress(float fraction)
{
    if(mProgressBar != nullptr)
    {
        mProgressBar->SetProgress(fraction);
    }
}

void GK3UI::HideProgressBar()
{
    if(mProgressBar != nullptr)
    {
        mProgressBar->Hide();
    }
}

void GK3UI::ShowPauseScreen()
{
    if(mPauseScreen == nullptr)
    {
        mPauseScreen = new PauseScreen();
        mPauseScreen->SetIsDestroyOnLoad(false);
    }
    mPauseScreen->Show();
}

void GK3UI::ShowQuitPopup()
{
    if(mQuitPopup == nullptr)
    {
        mQuitPopup = new QuitPopup();
        mQuitPopup->SetIsDestroyOnLoad(false);
    }
    mQuitPopup->Show();
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

void GK3UI::ShowFingerprintInterface(const std::string& nounName)
{
    if(mFingerprintScreen == nullptr)
    {
        mFingerprintScreen = new FingerprintScreen();
        mFingerprintScreen->SetIsDestroyOnLoad(false);
    }
    mFingerprintScreen->Show(nounName);
}

void GK3UI::ShowBinocsOverlay()
{
    if(mBinocsOverlay == nullptr)
    {
        mBinocsOverlay = new BinocsOverlay();
        mBinocsOverlay->SetIsDestroyOnLoad(false);
    }
    mBinocsOverlay->Show();
}

void GK3UI::ShowGPSOverlay()
{
    if(mGPSOverlay == nullptr)
    {
        mGPSOverlay = new GPSOverlay();
        mGPSOverlay->SetIsDestroyOnLoad(false);
    }
    mGPSOverlay->Show();
}

void GK3UI::HideGPSOverlay()
{
    if(mGPSOverlay != nullptr)
    {
        mGPSOverlay->Hide();
    }
}

bool GK3UI::IsAnyKeyPressedOutsideTextInputAndConsole()
{
    // For a few UIs, any keyboard input cancels the current UI (e.g. action bar, option bar).
    // However, there are a few exceptions:
    // 1) If a text input field is focused, then the keyboard input doesn't cancel the UI.
    // 2) The ~ (GRAVE) key opens/closes the console, so it is ignored.
    //TODO: This technically also cancels any key pressed WHILE ~ is held down. Close enough, but can be improved!
    return !gInputManager.IsTextInput() && gInputManager.IsAnyKeyLeadingEdge() && !gInputManager.IsKeyPressed(SDL_SCANCODE_GRAVE);
}

bool GK3UI::CanExitScreen(const Layer& layer)
{
    return gLayerManager.IsTopLayer(&layer) &&          // must be on the given layer
        !gActionManager.IsActionPlaying() &&            // must not be playing an action
        !gActionManager.IsSkippingCurrentAction();      // must not be skipping an action
}
