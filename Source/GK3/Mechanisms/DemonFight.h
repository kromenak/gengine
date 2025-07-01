//
// Clark Kromenaker
//
// A helper script to manage behavior during the demon fight in the last scene of the game.
//
#pragma once
#include "Actor.h"

class DemonFight : public Actor
{
public:
    DemonFight();

protected:
    void OnLateUpdate(float deltaTime) override;
};