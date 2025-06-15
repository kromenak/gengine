//
// Clark Kromenaker
//
// This screen just displays a "Paused" graphic, and pauses the underlying scene.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class PauseScreen : public Actor
{
public:
    PauseScreen();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    Layer mLayer;

    // A timer before pressing "P" will allow you to dismiss this screen.
    // This is a very short timer, just to avoid dismissing the screen on the same frame you show it.
    float mInputDelayTimer = 0.0f;
};