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
class Animator;

class GasPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    GasPlayer(Actor* owner);
    
	void SetGas(GAS* gas) { mGas = gas; }
    
	void Play() { mNodeIndex = 0; mTimer = 0.0f; mPaused = false; }
    void Play(GAS* gas) { mGas = gas; Play(); }
	void Pause() { mPaused = true; }
	void Resume() { mPaused = false; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
    // The GAS asset being played.
    GAS* mGas = nullptr;
	
    // The current node index being played.
    int mNodeIndex = 0;
    
    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;
	
	// If true, gas player is paused.
	bool mPaused = true;
    
    void ProcessNextNode();
};

