//
// Clark Kromenaker
//
// A screen showing that you are entering a new timeblock.
//
#pragma once
#include "Actor.h"

#include <functional>

class Sequence;
class Timeblock;
class UIImage;

class TimeblockScreen : public Actor
{
public:
    TimeblockScreen();

    void Show(const Timeblock& timeblock, std::function<void()> callback);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The background image changes depending on the current timeblock.
    UIImage* mBackgroundImage = nullptr;

    // The onscreen text (showing like "Day 1 10AM") is actually a series of images.
    UIImage* mTextImage = nullptr;

    // Sequence and timer for the on-screen text animation.
    Sequence* mAnimSequence = nullptr;
    float mAnimTimer = 0.0f;

    // A timer to track how long to show this screen.
    float mScreenTimer = 0.0f;

    // A callback to execute when the screen is done showing.
    std::function<void()> mCallback;
};