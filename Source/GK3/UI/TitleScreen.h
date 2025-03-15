//
// Clark Kromenaker
//
// Title screen - shows game's title and options to start a new game, load, quit, etc.
//
#pragma once
#include "Actor.h"

class UIButton;

class TitleScreen : public Actor
{
public:
    TitleScreen();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Button references to support keyboard shortcuts.
    UIButton* mIntroButton = nullptr;
    UIButton* mPlayButton = nullptr;
    UIButton* mRestoreButton = nullptr;
    UIButton* mQuitButton = nullptr;
};