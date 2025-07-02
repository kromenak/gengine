//
// Clark Kromenaker
//
// This popup asks you if you're sure you want to quit the game.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class UIButton;

class QuitPopup : public Actor
{
public:
    QuitPopup();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    Layer mLayer;

    // The two possible button choices on this screen.
    UIButton* mYesButton = nullptr;
    UIButton* mNoButton = nullptr;
};