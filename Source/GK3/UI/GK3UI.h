//
// Clark Kromenaker
//
// A helper class for showing/hiding various GK3 UIs.
//
#pragma once
#include <functional>
#include <string>

class CaptionsOverlay;
class DrivingScreen;
class SceneTransitioner;
class Sidney;
class Timeblock;
class TimeblockScreen;
class TitleScreen;

class GK3UI
{
public:
    void ShowTitleScreen();
    void ShowTimeblockScreen(const Timeblock& timeblock, float timer, std::function<void()> callback);
    void ShowFinishedScreen();

    void ShowSceneTransitioner();
    void HideSceneTransitioner();

    void AddCaption(const std::string& caption, const std::string& speaker = "NOTLISTED");
    void FinishCaption(float delay = 1.0f);
    void HideAllCaptions();

    void ShowDrivingScreen(int followState = 0);
    bool FollowingOnDrivingScreen();

    Sidney* GetSidney();
    void ShowSidney();

private:
    TitleScreen* mTitleScreen = nullptr;
    TimeblockScreen* mTimeblockScreen = nullptr;

    SceneTransitioner* mSceneTransitioner = nullptr;

    CaptionsOverlay* mCaptionsOverlay = nullptr;

    DrivingScreen* mDrivingScreen = nullptr;

    Sidney* mSidney = nullptr;
};

extern GK3UI gGK3UI;