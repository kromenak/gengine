//
// Clark Kromenaker
//
// A helper class for showing/hiding various GK3 UIs.
//
#pragma once
#include <functional>
#include <string>

#include "VideoPlayer.h"

class BinocsOverlay;
class CaptionsOverlay;
class ConfirmPopup;
class DeathScreen;
class DrivingScreen;
class FingerprintScreen;
class GPSOverlay;
class HelpScreen;
class Layer;
class PauseScreen;
class ProgressBar;
class QuitPopup;
class SaveLoadScreen;
class SceneTransitioner;
class Sidney;
class Timeblock;
class TimeblockScreen;
class TitleScreen;
class VideoPlayer;

class GK3UI
{
public:
    void ShowTitleScreen();
    void HideTitleScreen();

    void ShowTimeblockScreen(const Timeblock& timeblock, float timer, bool loadingSave, const std::function<void()>& callback);
    void ShowDeathScreen();
    void ShowFinishedScreen();

    void ShowSaveScreen();
    void ShowLoadScreen();

    ProgressBar* ShowGenericProgressBar();
    ProgressBar* ShowSaveProgressBar();
    ProgressBar* ShowLoadProgressBar();
    void SetProgressBarProgress(float fraction);
    void HideProgressBar();

    void ShowPauseScreen();
    void ShowHelpScreen();
    void ShowQuitPopup();
    void ShowConfirmPopup(const std::string& message, const std::function<void(bool)>& callback);

    void ShowSceneTransitioner();
    void HideSceneTransitioner();

    void AddCaption(const std::string& caption, const std::string& speaker = "NOTLISTED");
    void FinishCaption(float delay = 1.0f);
    void HideAllCaptions();

    void ShowDrivingScreen(int followState = 0);
    bool FollowingOnDrivingScreen();
    bool IsOnDrivingScreen();
    void FlashDrivingScreenLocation(const std::string& locationCode);

    Sidney* GetSidney();
    void ShowSidney();

    void ShowFingerprintInterface(const std::string& nounName);
    void ShowBinocsOverlay();

    void ShowGPSOverlay();
    void HideGPSOverlay();

    VideoPlayer* GetVideoPlayer();

    bool IsAnyKeyPressedOutsideTextInputAndConsole();
    bool CanExitScreen(const Layer& layer);

    bool IsOnExitableScreen() const;
    void ExitCurrentScreen();
    void HideAllScreens();

private:
    TitleScreen* mTitleScreen = nullptr;

    TimeblockScreen* mTimeblockScreen = nullptr;
    DeathScreen* mDeathScreen = nullptr;

    SaveLoadScreen* mSaveLoadScreen = nullptr;

    ProgressBar* mProgressBar = nullptr;

    PauseScreen* mPauseScreen = nullptr;
    HelpScreen* mHelpScreen = nullptr;
    QuitPopup* mQuitPopup = nullptr;
    ConfirmPopup* mConfirmPopup = nullptr;

    SceneTransitioner* mSceneTransitioner = nullptr;

    CaptionsOverlay* mCaptionsOverlay = nullptr;

    DrivingScreen* mDrivingScreen = nullptr;

    Sidney* mSidney = nullptr;

    FingerprintScreen* mFingerprintScreen = nullptr;
    BinocsOverlay* mBinocsOverlay = nullptr;
    GPSOverlay* mGPSOverlay = nullptr;

    VideoPlayer* mVideoPlayer = nullptr;
};

extern GK3UI gGK3UI;