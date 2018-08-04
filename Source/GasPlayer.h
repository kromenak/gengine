//
// GasPlayer.cpp
//
// Clark Kromenaker
//
// A component that can play a GAS file.
//
#pragma once
#include "Component.h"

class GAS;

class GasPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    GasPlayer(Actor* owner);
    
    void Update(float deltaTime) override;
    
    void SetGas(GAS* gas) { mGas = gas; mNodeIndex = 0; }
    
private:
    // The GAS asset being played.
    GAS* mGas = nullptr;
    
    // The current node index being played.
    int mNodeIndex = 0;
};

