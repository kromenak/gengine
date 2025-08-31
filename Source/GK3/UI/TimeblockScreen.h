//
// Clark Kromenaker
//
// A screen showing that you are entering a new timeblock.
//
#pragma once
#include "Actor.h"

#include <functional>

#include "LayerManager.h"
#include "WaitForNoInput.h"

class Sequence;
class Timeblock;
class UIButton;
class UIImage;

class TimeblockScreen : public Actor
{
public:
    TimeblockScreen();

    void Show(const Timeblock& timeblock, float timer, bool loadingSave, const std::function<void()>& callback);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    Layer mLayer;

    // The background image changes depending on the current timeblock.
    UIImage* mBackgroundImage = nullptr;

    // The onscreen text (showing like "Day 1 10AM") is actually a series of images.
    UIImage* mTextImage = nullptr;

    // Buttons for continuing and saving.
    // If a timer is passed in, these buttons are hidden. If no timer, this screen stays up until a "Continue" is pressed.
    UIButton* mContinueButton = nullptr;
    UIButton* mSaveButton = nullptr;

    // Sequence and timer for the on-screen text animation.
    Sequence* mAnimSequence = nullptr;
    float mAnimTimer = 0.0f;

    // A timer to track how long to show this screen.
    float mScreenTimer = 0.0f;

    // A callback to execute when the screen is done showing.
    std::function<void()> mCallback;

    // Helper for dealing with shortcut keys correctly.
    WaitForNoInput mWaitForNoInput;
};