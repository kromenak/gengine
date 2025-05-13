//
// Clark Kromenaker
//
// A screen that appears when you die.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class DeathScreen : public Actor
{
public:
    DeathScreen();

    void Show();
    void Hide();

private:
    // This screen's layer.
    Layer mLayer;

    void OnRetryButtonPressed();
};
