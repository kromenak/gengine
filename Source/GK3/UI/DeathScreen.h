//
// Clark Kromenaker
//
// A screen that appears when you die.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class UIButton;
class UICanvas;
class UIImage;

class DeathScreen : public Actor
{
public:
    DeathScreen();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // This screen's layer.
    Layer mLayer;

    // This screen's canvas.
    UICanvas* mCanvas = nullptr;

    // The background image.
    UIImage* mBackgroundImage = nullptr;

    // The buttons on the screen.
    UIButton* mRetryButton = nullptr;
    UIButton* mRestoreButton = nullptr;
    UIButton* mQuitButton = nullptr;

    void OnRetryButtonPressed();
    void RefreshUIScaling();
};
