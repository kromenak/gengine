//
// Clark Kromenaker
//
// A component that can play a GAS file.
//
#pragma once
#include "Component.h"

#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

class Animation;
class Animator;
class GAS;
struct ScenePosition;
struct WhenNearGasNode;

class GasPlayer : public Component
{
    TYPEINFO_SUB(GasPlayer, Component);
public:
    GasPlayer(Actor* owner);

    void Play(GAS* gas);
    void Pause();
    void Resume();
    void Stop(const std::function<void()>& callback = nullptr);
    void StopAndCancelAnims();

    void Interrupt(bool forTalk, const std::function<void()>& callback);

    // Below here: GAS Node Helpers
    void SetVar(char var, int value) { mVariables[var - 'A'] = value; }
    int GetVar(char var) { return mVariables[var - 'A']; }

    void SetNodeIndex(int index);
    void NextNode(int forExecutionCounter);

    void StartAnimation(Animation* anim, const std::function<void()>& finishCallback = nullptr);

    void SetInterruptPosition(const ScenePosition* interruptPosition) { mInterruptConfig.position = interruptPosition; }
    void SetInterruptAnimation(Animation* animation) { mInterruptConfig.animation = animation; }
    void SetCleanup(Animation* animNeedingCleanup, Animation* animDoingCleanup) { mInterruptConfig.cleanups[animNeedingCleanup] = animDoingCleanup; }
    void SetNewIdleOnAction(GAS* newIdle) { mInterruptConfig.newIdle = newIdle; }
    void SetInterruptClearFlag(const std::string& clearFlag) { mInterruptConfig.clearFlag = clearFlag; }

    void SetTalkInterruptPosition(const ScenePosition* interruptPosition) { mTalkInterruptConfig.position = interruptPosition; }
    void SetTalkInterruptAnimation(Animation* animation) { mTalkInterruptConfig.animation = animation; }
    void SetTalkCleanup(Animation* animNeedingCleanup, Animation* animDoingCleanup) { mTalkInterruptConfig.cleanups[animNeedingCleanup] = animDoingCleanup; }
    void SetNewIdleOnTalkAction(GAS* newIdle) { mTalkInterruptConfig.newIdle = newIdle; }
    void SetTalkInterruptClearFlag(const std::string& clearFlag) { mTalkInterruptConfig.clearFlag = clearFlag; }

    void AddDistanceCondition(WhenNearGasNode* node);

    int GetExecutionCounter() const { return mExecutionCounter; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The GAS being played.
    GAS* mGas = nullptr;

    // A GAS consists of one or more nodes.
    // This is the current node index in the GAS being played.
    int mNodeIndex = -1;

    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;

    // If true, the player is paused. No nodes will execute until unpaused.
    bool mPaused = true;

    // If true, a next node request was received while the player was paused.
    // We'll remember that and execute it when/if the player unpauses.
    bool mReceivedNextNodeRequestWhilePaused = false;

    // Counts how many node executions have occurred.
    // This is particularly important so that, when we receive a callback, we can ensure it's still relevant to the player.
    int mExecutionCounter = 0;

    // Variables for running autoscript.
    // Note that autoscript variables MUST be named uppercase A-Z.
    static const int kMaxVariables = 26;
    int mVariables[kMaxVariables] = { 0 };

    // The current GAS animation being played.
    // Needs to be saved here so we know whether we need to do a cleanup or not.
    Animation* mCurrentAnimation = nullptr;

    // Some actions (like performing cleanups) look better if you wait for the current animation to finish before doing them.
    // These vars keep track of whether an anim is still playing, and if so, an optional callback to execute when the anim is done.
    bool mCurrentAnimationStillPlaying = false;
    std::function<void()> mCurrentAnimationDoneCallback = nullptr;

    // Nodes that do distance condition checks to determine whether to go to some label/index in the current autoscript.
    // The "bool" is to hold if the condition is currently true - the condition only triggers when going from false to true.
    std::vector<std::pair<WhenNearGasNode*, bool>> mDistanceConditionNodes;

    // An "interrupt" is when an Actor's playing autoscript is interrupted.
    // Autoscripts typically represent idle/passive behavior, so an action being performed on the autoscript owner "interrupts" the autoscript.
    struct InterruptConfig
    {
        // Autoscripts play a variety of animations, sometimes putting Actors into odd poses/situations.
        // But when an autoscript is interrupted, we want the Actor to go back to their default state.
        // "Cleanup" animations map on animation to the next one needed to "clean up" the current animation situation.
        // As long as the current animation is a key in this map, we are not yet fully "cleaned up"!
        std::unordered_map<Animation*, Animation*> cleanups;

        // An optional position in the scene to walk to when interrupted.
        // Once walked to the position, also an optional animation to play.
        const ScenePosition* position = nullptr;
        Animation* animation = nullptr;

        // If set the Actor's idle fidget will change to this GAS at the end of the interrupt.
        GAS* newIdle = nullptr;

        // If set, this flag is cleared when an interrupt occurs.
        std::string clearFlag;
    };

    // Interrupt configs for "normal" and "talk" scenarios.
    // In "talk" situations, the talk interrupt is used instead, UNLESS it isn't defined, in which case we fall back on the "normal interrupt config.
    InterruptConfig mInterruptConfig;
    InterruptConfig mTalkInterruptConfig;

    void ProcessNextNode();

    void CheckDistanceConditions();

    void WaitForCurrentAnimationToFinish(const std::function<void()>& callback);
    void PerformCleanups(bool forTalk, const std::function<void()>& callback);
    void WalkToInterruptPos(bool forTalk, const std::function<void()>& callback);
    void PlayInterruptAnimation(bool forTalk, const std::function<void()>& callback);
    void SetNewIdle(bool forTalk, const std::function<void()>& callback);
    void ClearFlag(bool forTalk);
};
