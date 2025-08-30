#include "GasPlayer.h"

#include "ActionManager.h"
#include "Actor.h"
#include "Animation.h"
#include "Animator.h"
#include "GameProgress.h"
#include "GAS.h"
#include "GasNodes.h"
#include "GKActor.h"
#include "SceneManager.h"
#include "Walker.h"

TYPEINFO_INIT(GasPlayer, Component, 9)
{

}

GasPlayer::GasPlayer(Actor* owner) : Component(owner)
{

}

void GasPlayer::Play(GAS* gas)
{
    // Ignore request to play GAS that is already playing.
    if(gas == mGas) { return; }

    // Set GAS.
    mGas = gas;

    // Increment execution counter so that any anim callbacks from current GAS nodes are invalidated.
    // Since the GAS has changed, those callbacks aren't valid anymore.
    ++mExecutionCounter;

    // Treat a null GAS as a request to stop.
    if(mGas == nullptr)
    {
        Stop();
    }

    // Reset execution state.
    mNodeIndex = -1;
    mTimer = 0.0f;
    mPaused = false;
    mReceivedNextNodeRequestWhilePaused = false;

    // Reset animation playback state.
    mCurrentAnimation = nullptr;
    mCurrentAnimationStillPlaying = false;
    mCurrentAnimationDoneCallback = nullptr;

    // Reset all vars.
    for(int i = 0; i < kMaxVariables; i++)
    {
        mVariables[i] = 0;
    }

    // Clear distance condition nodes.
    mDistanceConditionNodes.clear();

    // Reset interrupt vars.
    mInterruptConfig = InterruptConfig();
    mTalkInterruptConfig = InterruptConfig();

    // Execute at least the first node immediately.
    // We don't want to wait until Update is called, since that may not happen until next frame (depending on order of operations).
    // And that could result in a single rendered frame with the wrong anim for example.
    ProcessNextNode();
}

void GasPlayer::Pause()
{
    if(!mPaused)
    {
        mPaused = true;
        mReceivedNextNodeRequestWhilePaused = false;
    }
}

void GasPlayer::Resume()
{
    if(mPaused)
    {
        // No longer paused.
        mPaused = false;

        // Process the next node if we got that request while paused.
        if(mReceivedNextNodeRequestWhilePaused)
        {
            mReceivedNextNodeRequestWhilePaused = false;
            ProcessNextNode();
        }
    }
}

void GasPlayer::Stop(const std::function<void()>& callback)
{
    // Clear GAS script, which stops any more nodes from executing.
    mGas = nullptr;

    // Perform cleanups.
    //TODO: if you stop a fidget due to an error (tried to set a null fidget), it's supposed to IMMEDIATELY STOP (leaving any anims mid-play) - no cleanups.
    PerformCleanups(false, callback);
}

void GasPlayer::StopAndCancelAnims()
{
    mGas = nullptr;

    if(mCurrentAnimation != nullptr)
    {
        gSceneManager.GetScene()->GetAnimator()->Stop(mCurrentAnimation, true);
        mCurrentAnimation = nullptr;
        mCurrentAnimationStillPlaying = false;
        mCurrentAnimationDoneCallback = nullptr;
    }
}

void GasPlayer::Interrupt(bool forTalk, const std::function<void()>& callback)
{
    // Start a manual action. This makes the game non-interactive for a bit as the interruption occurs.
    gActionManager.StartManualAction();

    // We're interrupting playback, so pause.
    // This also ensures no additional GAS nodes execute during cleanups/walks.
    Pause();

    // Play any cleanups. This moves the Actor back to a default pose if in some weird pose.
    PerformCleanups(forTalk, [this, forTalk, callback](){

        // If there's a clear flag, clear it.
        // Best to do this before walk code and new idle might clear/change the playing GAS.
        ClearFlag(forTalk);

        // Then, walk to interrupt position and play any interrupt animation.
        WalkToInterruptPos(forTalk, [this, forTalk, callback](){

            // Finally, change idle gas, if any is specified.
            SetNewIdle(forTalk, callback);

            // It's likely that any walk code or new idle code already resumed the player.
            // But in case there was no walk or new idle, resume playback manually.
            Resume();

            // All done - clear the manual action.
            gActionManager.FinishManualAction();
        });
    });
}

void GasPlayer::SetNodeIndex(int index)
{
    // This function is meant to be called from some other node that wants to change this GAS player's current node.
    // For example, IF, GOTO, LOOP, WHENNEAR, etc.
    // We must use "index - 1" b/c ProcessNextNode will increment before executing, so we want it to increment to mNodes[index].
    mNodeIndex = index - 1;
}

void GasPlayer::NextNode(int forExecutionCounter)
{
    // If the execution counters don't match, it means this GAS Player started doing something else, and is no longer waiting for this callback.
    // So...ignore it!
    if(mExecutionCounter == forExecutionCounter)
    {
        ProcessNextNode();
    }
}

void GasPlayer::StartAnimation(Animation* anim, const std::function<void()>& finishCallback)
{
    // Save playing animation so we know whether we need to do cleanups when stopping the autoscript.
    mCurrentAnimation = anim;

    // This anim is starting to play.
    mCurrentAnimationStillPlaying = true;

    // Play the thing.
    AnimParams animParams;
    animParams.animation = anim;
    animParams.fromAutoScript = true;
    gSceneManager.GetScene()->GetAnimator()->Start(animParams, [this, finishCallback](){

        // Anim is no longer playing.
        mCurrentAnimationStillPlaying = false;

        // If anyone was waiting for this anim to complete, let them know it's done.
        if(mCurrentAnimationDoneCallback != nullptr)
        {
            auto callback = mCurrentAnimationDoneCallback;
            mCurrentAnimationDoneCallback = nullptr;
            callback();
        }

        // Let caller know the animation is done too.
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
    });
}

void GasPlayer::AddDistanceCondition(WhenNearGasNode* node)
{
    // Add distance condition.
    mDistanceConditionNodes.push_back(std::make_pair(node, false));

    // There's a chance that a distance condition is met *immediately* when it's added.
    // In that case, we should immediately detect it, and trigger a skip to the appropriate node index.
    CheckDistanceConditions();
}

void GasPlayer::OnUpdate(float deltaTime)
{
    if(mGas == nullptr || mPaused) { return; }

    // Decrement timer. When it gets to zero, we move onto the next node.
    if(mTimer > 0.0f)
    {
        mTimer -= deltaTime;
        if(mTimer <= 0.0f)
        {
            ProcessNextNode();
        }
    }

    // See if any distance condition has been met.
    CheckDistanceConditions();
}

void GasPlayer::ProcessNextNode()
{
    // If GAS is null, or has no nodes, we can't do anything.
    if(mGas == nullptr || mGas->GetNodeCount() == 0)
    {
        mTimer = 0.0f;
        return;
    }

    // We're paused, so we can't process any nodes.
    // Remember that we got this request for when we unpause though.
    if(mPaused)
    {
        mReceivedNextNodeRequestWhilePaused = true;
        return;
    }

    // We at least want to execute one node - the next one.
    // But if multiple nodes have zero duration, we should execute them all at once.
    // So, keep looping until we either hit a non-zero-duration node, OR we execute too many nodes (500 is an arbitrary number - could be larger if needed).
    int executeCount = 0;
    while(executeCount < 500)
    {
        // It's possible for a node execution to null out the autoscript, so catch that and break out of this loop.
        if(mGas == nullptr)
        {
            break;
        }

        // First, update the node index and loop if necessary.
        ++mNodeIndex;
        mNodeIndex %= mGas->GetNodeCount();

        // Grab the node from the list.
        GasNode* node = mGas->GetNode(mNodeIndex);
        if(node == nullptr)
        {
            mTimer = 0.0f;
            return;
        }

        // Execute the node, which returns amount of time it will take to perform the node.
        // Note that a negative time can be returned here - this means the node will use the callback mechanism (NextNode()) instead of a timer.
        ++executeCount;
        ++mExecutionCounter;
        mTimer = node->Execute(this);

        // This node set a non-zero timer duration, so we can break out of this loop for now.
        if(mTimer != 0.0f)
        {
            break;
        }
    }
}

void GasPlayer::CheckDistanceConditions()
{
    // Check distance conditions. If one goes from false to true, the execution index is changed.
    for(auto& condition : mDistanceConditionNodes)
    {
        bool conditionMet = condition.first->CheckCondition(this);
        if(!condition.second && conditionMet)
        {
            condition.second = true;
            SetNodeIndex(condition.first->index);
        }
        else if(condition.second && !conditionMet)
        {
            condition.second = false;
        }
    }
}

void GasPlayer::WaitForCurrentAnimationToFinish(const std::function<void()>& callback)
{
    // If an anim is playing, save callback to be called when the anim is done.
    // If no anim is playing, we can do the callback right away.
    if(mCurrentAnimationStillPlaying)
    {
        mCurrentAnimationDoneCallback = callback;
    }
    else
    {
        callback();
    }
}

void GasPlayer::PerformCleanups(bool forTalk, const std::function<void()>& callback)
{
    // Get the correct cleanups set.
    // For talk cleanups, the talk interrupt cleanups take priority, but we fall back on normal interrupt cleanups if empty.
    std::unordered_map<Animation*, Animation*>* cleanups = nullptr;
    if(forTalk)
    {
        cleanups = &mTalkInterruptConfig.cleanups;
    }
    if(cleanups == nullptr || cleanups->empty())
    {
        cleanups = &mInterruptConfig.cleanups;
    }

    // See if the currently playing autoscript animation requires any cleanups.
    auto it = cleanups->find(mCurrentAnimation);
    if(it == cleanups->end())
    {
        // No cleanup anim found - we're done!
        if(callback != nullptr)
        {
            callback();
        }
        return;
    }

    // This is a bit hacky, but since we're playing a cleanup anim, increment execution counter so no callbacks from previous anims cause additional nodes to execute.
    ++mExecutionCounter;

    // Before playing the cleanup for the current animation, let the current animation actually finish.
    // This avoids some jumpy cuts between anims, and is required in one case for a cutscene to look correct (Buchelli/Wilkes in Dining Room, Day 1 6PM).
    Animation* cleanupAnim = it->second;
    WaitForCurrentAnimationToFinish([this, forTalk, callback, cleanupAnim](){

        // Ok, let's do the cleanup!
        // When the cleanup finishes, this function re-calls itself, until all cleanups are done.
        StartAnimation(cleanupAnim, [this, forTalk, callback](){
            PerformCleanups(forTalk, callback);
        });
    });
}

void GasPlayer::WalkToInterruptPos(bool forTalk, const std::function<void()>& callback)
{
    // Get the walker and have it walk to the interrupt pos.
    Walker* walker = GetOwner()->GetComponent<Walker>();
    if(walker != nullptr)
    {
        // Resolve which interrupt position to use.
        // The talk interrupt position is used for talk interrupts, but falls back on normal interrupt position if not set.
        const ScenePosition* interruptPos = nullptr;
        if(forTalk)
        {
            interruptPos = mTalkInterruptConfig.position;
        }
        if(interruptPos == nullptr)
        {
            interruptPos = mInterruptConfig.position;
        }

        // Walk to the position if we have any.
        if(interruptPos != nullptr)
        {
            walker->WalkToExact(interruptPos->position, interruptPos->heading, [this, forTalk, callback](){

                // Do interrupt anim after walk.
                PlayInterruptAnimation(forTalk, callback);
            });
            return;
        }
    }

    // If we get here, either there's no walker or there's no interrupt position.
    // In either case, just skip ahead to play an interrupt anim, if any.
    PlayInterruptAnimation(forTalk, callback);
}

void GasPlayer::PlayInterruptAnimation(bool forTalk, const std::function<void()>& callback)
{
    // Resolve which animation to play.
    // As with others, the talk interrupt animation is used for talk interrupts, but falls back on normal interrupt animation if not set.
    Animation* interruptAnimation = nullptr;
    if(forTalk)
    {
        interruptAnimation = mTalkInterruptConfig.animation;
    }
    if(interruptAnimation == nullptr)
    {
        interruptAnimation = mInterruptConfig.animation;
    }

    // Play an animation if we have it.
    if(interruptAnimation != nullptr)
    {
        // This is a bit hacky, but since we're playing a new anim, increment execution counter so no callbacks from previous anims cause additional nodes to execute.
        ++mExecutionCounter;
        StartAnimation(interruptAnimation, callback);
    }
    else
    {
        if(callback != nullptr)
        {
            callback();
        }
    }
}

void GasPlayer::SetNewIdle(bool forTalk, const std::function<void()>& callback)
{
    // Resolve new GAS to use.
    GAS* newIdleGas = nullptr;
    if(forTalk)
    {
        newIdleGas = mTalkInterruptConfig.newIdle;
    }
    else
    {
        newIdleGas = mInterruptConfig.newIdle;
    }

    // If we've got a new idle GAS, set it on our owner.
    if(newIdleGas != nullptr && GetOwner()->IsA<GKActor>())
    {
        GKActor* gkOwner = static_cast<GKActor*>(GetOwner());
        gkOwner->SetIdleFidget(newIdleGas);
    }

    // Whether we set one or not, do the callback.
    if(callback != nullptr)
    {
        callback();
    }
}

void GasPlayer::ClearFlag(bool forTalk)
{
    gGameProgress.ClearFlag(forTalk ? mTalkInterruptConfig.clearFlag : mInterruptConfig.clearFlag);
}
