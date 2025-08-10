//
// Clark Kromenaker
//
// Shows when you finish the game. Says "THE END" and exits to the desktop after some time.
//
#pragma once
#include "Actor.h"

class UICanvas;
class UIImage;

class FinishedScreen : public Actor
{
public:
    FinishedScreen();

    void Show();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The canvas for this screen.
    UICanvas* mCanvas = nullptr;

    // The main thing on this screen is a big image.
    UIImage* mBackgroundImage = nullptr;

    // To avoid accidentally dismissing the screen (and quitting the game) right when the screen shows...
    // Wait a short amount of time before checking for inputs to quit the game.
    const float mInputDelayDuration = 2.0f;
    float mInputDelayTimer = 0.0f;

    bool mWaitingForNoInput = false;

    void RefreshUIScaling();
};
