//
// GasPlayer.cpp
//
// Clark Kromenaker
//
// A component that can play a GAS file.
//
#pragma once
#include "Component.h"

#include <unordered_map>

#include "GasNodes.h"

class Animation;
class Animator;
class GAS;
class ScenePosition;

class GasPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    GasPlayer(Actor* owner);
    
    void Play(GAS* gas);
	void Pause() { mPaused = true; }
	void Resume() { mPaused = false; }
    void Stop();

    // For use by GAS nodes primarily
    void SetIndex(int index) { mNodeIndex = index - 1; }

    void SetVar(char var, int value) { mVariables[var - 'A'] = value; }
    int GetVar(char var) { return mVariables[var - 'A']; }

    void NextNode();

    void SetInterruptPosition(const ScenePosition* interruptPosition) { mInterruptPosition = interruptPosition; }
    void SetCleanup(Animation* animNeedingCleanup, Animation* animDoingCleanup) { mCleanupAnims[animNeedingCleanup] = animDoingCleanup; }

    void SetTalkInterruptPosition(const ScenePosition* interruptPosition) { mTalkInterruptPosition = interruptPosition; }
    void SetTalkCleanup(Animation* animNeedingCleanup, Animation* animDoingCleanup) { mTalkCleanupAnims[animNeedingCleanup] = animDoingCleanup; }

    void AddWhenNoLongerNearNode(WhenNoLongerNearGasNode* node) { mActiveWhenNoLongerNearNodes.push_back(node); }

protected:
	void OnUpdate(float deltaTime) override;
	
private:
    // The GAS asset being played.
    GAS* mGas = nullptr;
	
    // The current node index being played.
    int mNodeIndex = -1;
    
    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;
	
	// If true, gas player is paused.
	bool mPaused = true;

    // Variables for running autoscript.
    // Note that autoscript variables MUST be named uppercase A-Z.
    static const unsigned char kMaxVariables = 26;
    int mVariables[kMaxVariables] = { 0 };

    // An "interrupt position" to use if Ego requests to interact while in the middle of a walk.
    const ScenePosition* mInterruptPosition = nullptr;

    // Autoscripts may cause actors to play a variety of animations, putting them in odd situations.
    // But when someone wants the actor's attention, we want them to go back to their default state first.
    // "Cleanup" animations map one animation to the next one to play to continue "cleaning up" the current animation situation.
    // As long as the current animation is a key in this map, we are not yet fully "cleaned up"!
    std::unordered_map<Animation*, Animation*> mCleanupAnims;

    // Similar to previous interrupt position, but only used for interrupts relating to talking.
    // If not set, the normal interrupt position is used.
    const ScenePosition* mTalkInterruptPosition = nullptr;

    // Again, just like the above, but only used for talk interrupts.
    // If no talk interrupts are provided, the normal cleanups are used.
    std::unordered_map<Animation*, Animation*> mTalkCleanupAnims;

    std::vector<WhenNoLongerNearGasNode*> mActiveWhenNoLongerNearNodes;

    void ProcessNextNode();
};