//
// GasPlayer.cpp
//
// Clark Kromenaker
//
#include "GasPlayer.h"

TYPE_DEF_CHILD(Component, GasPlayer);

GasPlayer::GasPlayer(Actor* owner) : Component(owner)
{
    
}

void GasPlayer::Update(float deltaTime)
{
    if(mGas == nullptr) { return; }
    
    
}
