//
// Clark Kromenaker
//
// This popup asks you if you're sure you want to quit the game.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class QuitPopup : public Actor
{
public:
    QuitPopup();

    void Show();
    void Hide();

private:
    Layer mLayer;
};