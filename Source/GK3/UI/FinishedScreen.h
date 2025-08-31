//
// Clark Kromenaker
//
// Shows when you finish the game. Says "THE END" and exits to the desktop after some time.
//
#pragma once
#include "Actor.h"

#include "WaitForNoInput.h"

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

    // Helper to only dismiss this screen after no inputs are detected.
    WaitForNoInput mWaitForNoInput;

    void RefreshUIScaling();
};
