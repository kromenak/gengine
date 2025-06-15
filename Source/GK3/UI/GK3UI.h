//
// Clark Kromenaker
//
// A helper class for showing/hiding various GK3 UIs.
//
#pragma once
#include <functional>
#include <string>

class BinocsOverlay;
class CaptionsOverlay;
class DeathScreen;
class DrivingScreen;
class FingerprintScreen;
class GPSOverlay;
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

class GK3UI
{
public:
    void ShowTitleScreen();
    void HideTitleScreen();

    void ShowTimeblockScreen(const Timeblock& timeblock, float timer, std::function<void()> callback);
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
    void ShowQuitPopup();

    void ShowSceneTransitioner();
    void HideSceneTransitioner();

    void AddCaption(const std::string& caption, const std::string& speaker = "NOTLISTED");
    void FinishCaption(float delay = 1.0f);
    void HideAllCaptions();

    void ShowDrivingScreen(int followState = 0);
    bool FollowingOnDrivingScreen();

    Sidney* GetSidney();
    void ShowSidney();

    void ShowFingerprintInterface(const std::string& nounName);
    void ShowBinocsOverlay();

    void ShowGPSOverlay();
    void HideGPSOverlay();

    bool IsAnyKeyPressedOutsideTextInputAndConsole();
    bool CanExitScreen(const Layer& layer);

private:
    TitleScreen* mTitleScreen = nullptr;

    TimeblockScreen* mTimeblockScreen = nullptr;
    DeathScreen* mDeathScreen = nullptr;

    SaveLoadScreen* mSaveLoadScreen = nullptr;

    ProgressBar* mProgressBar = nullptr;

    PauseScreen* mPauseScreen = nullptr;
    QuitPopup* mQuitPopup = nullptr;

    SceneTransitioner* mSceneTransitioner = nullptr;

    CaptionsOverlay* mCaptionsOverlay = nullptr;

    DrivingScreen* mDrivingScreen = nullptr;

    Sidney* mSidney = nullptr;

    FingerprintScreen* mFingerprintScreen = nullptr;
    BinocsOverlay* mBinocsOverlay = nullptr;
    GPSOverlay* mGPSOverlay = nullptr;
};

extern GK3UI gGK3UI;