//
// GKActor.cpp
//
// Clark Kromenaker
//
// An "actor" in the GK3 system. An actor is considered to be
// one of the characters - like Gabe, Grace, Mosely. An actor is an entity
// that can walk around, animate, perform facial expressions and eye contact,
// play sound effects, etc.
//
// There are definitely "tiers" of actors - like, Gabe is an actor, but so
// are the "cat" and "chicken". So, maybe we need a subclass for humanoids at some point.
//
#pragma once
#include "Actor.h"

class GAS;
class GasPlayer;
class MeshRenderer;

class GKActor : public Actor
{
public:
    enum class State
    {
        Idle,
        Talk,
        Listen
    };
    
    GKActor();
    
    void SetIdleGas(GAS* gas) { mIdleGas = gas; }
    void SetTalkGas(GAS* gas) { mTalkGas = gas; }
    void SetListenGas(GAS* gas) { mListenGas = gas; }
    
    void SetState(State state);
    
    MeshRenderer* GetMeshRenderer() { return mMeshRenderer; }
    
private:
    // Actor's current state.
    State mState = State::Idle;
    
    // Player for GAS logic.
    GasPlayer* mGasPlayer = nullptr;
    
    // Mesh renderer.
    MeshRenderer* mMeshRenderer = nullptr;
    
    // GAS scripts to use when actor is idle, talking, or listening.
    GAS* mIdleGas = nullptr;
    GAS* mTalkGas = nullptr;
    GAS* mListenGas = nullptr;
};
