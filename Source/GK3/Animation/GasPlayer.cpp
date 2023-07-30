#include "GasPlayer.h"

#include "Animator.h"
#include "GAS.h"
#include "GasNodes.h"
#include "SceneManager.h"

TYPE_DEF_CHILD(Component, GasPlayer);

GasPlayer::GasPlayer(Actor* owner) : Component(owner)
{
    
}

void GasPlayer::Play(GAS* gas)
{
    // Ignore request to play GAS that is already playing.
    if(gas == mGas) { return; }

    // Set GAS.
    mGas = gas;

    // Treat a null GAS as a request to stop.
    if(mGas == nullptr)
    {
        Stop();
    }

    // Reset execution state.
    mNodeIndex = -1;
    mTimer = 0.0f;
    mPaused = false;

    // Reset all vars.
    for(int i = 0; i < kMaxVariables; i++)
    {
        mVariables[i] = 0;
    }

    // Reset other state vars.
    mInterruptPosition = nullptr;
    mCleanupAnims.clear();

    mTalkInterruptPosition = nullptr;
    mTalkCleanupAnims.clear();

    mDistanceConditionNodes.clear();
}

void GasPlayer::Stop(std::function<void()> callback)
{
    // Save stop callback - we'll call this after all cleanups have completed.
    mStopCallback = callback;

    // Clear GAS script, which stops any more nodes from executing.
    mGas = nullptr;

    // Perform cleanups.
    //TODO: if you stop a fidget due to an error (tried to set a null fidget), it's supposed to IMMEDIATELY STOP (leaving any anims mid-play) - no cleanups.
    PerformCleanups();
}

void GasPlayer::SetNodeIndex(int index)
{
    // This function is meant to be called from some other node that wants to change this GAS player's current node.
    // For example, IF, GOTO, LOOP, WHENNEAR, etc.
    // We must use "index - 1" b/c ProcessNextNode will increment before executing, so we want it to increment to mNodes[index].
    mNodeIndex = index - 1;
}

void GasPlayer::NextNode()
{
    ProcessNextNode();
}

void GasPlayer::StartAnimation(Animation* anim, std::function<void()> finishCallback)
{
    // Save playing animation so we know whether we need to do cleanups when stopping the autoscript.
    //TODO: This _might_ not be a good way to do this b/c the animation system might choose to NOT play a GAS animation b/c some other higher priority anim is playing.
    //TODO: In that case, no cleanups would be necessary. So, it might be better to somehow query the Animator or VertexAnimator to determine whether a cleanup is needed.
    mCurrentAnimation = anim;

    // Play the thing.
    AnimParams animParams;
    animParams.animation = anim;
    animParams.fromAutoScript = true;
    animParams.finishCallback = finishCallback;
    gSceneManager.GetScene()->GetAnimator()->Start(animParams);
}

void GasPlayer::OnUpdate(float deltaTime)
{
    if(mGas == nullptr) { return; }
    if(mPaused) { return; }

    // Decrement timer. When it gets to zero, we move onto the next node.
    if(mTimer >= 0.0f)
    {
        mTimer -= deltaTime;
        if(mTimer <= 0.0f)
        {
            ProcessNextNode();
        }
    }

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

void GasPlayer::ProcessNextNode()
{
    // If GAS is null, or has no nodes, we can't do anything.
    if(mGas == nullptr || mGas->GetNodeCount() == 0)
    {
        mTimer = 0.0f;
        return;
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
    mTimer = node->Execute(this);
}

void GasPlayer::PerformCleanups()
{
    //TODO: Handle normal vs. talk cleanups.

    // See if the currently playing autoscript animation requires any cleanups.
    auto it = mCleanupAnims.find(mCurrentAnimation);
    if(it == mCleanupAnims.end())
    {
        // No cleanup anim found - we're done!
        if(mStopCallback != nullptr)
        {
            auto callback = mStopCallback;
            mStopCallback = nullptr;
            callback();
        }
        return;
    }

    // Ok, let's do the cleanup!
    // When the cleanup finishes, this function re-calls itself, until all cleanups are done.
    //std::cout << "Performing cleanup " << it->second->GetName() << " for anim " << it->first->GetName() << std::endl;
    StartAnimation(it->second, std::bind(&GasPlayer::PerformCleanups, this));
}
