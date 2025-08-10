//
// Clark Kromenaker
//
// Title screen - shows game's title and options to start a new game, load, quit, etc.
//
#pragma once
#include "Actor.h"

class UICanvas;
class UIButton;
class UIImage;

class TitleScreen : public Actor
{
public:
    TitleScreen();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The canvas for this screen.
    UICanvas* mCanvas = nullptr;

    // The background image.
    UIImage* mBackgroundImage = nullptr;

    // Button references to support keyboard shortcuts.
    UIButton* mIntroButton = nullptr;
    UIButton* mPlayButton = nullptr;
    UIButton* mRestoreButton = nullptr;
    UIButton* mQuitButton = nullptr;

    void RefreshUIScaling();
};