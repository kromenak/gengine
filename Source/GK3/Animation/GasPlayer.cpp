#include "GasPlayer.h"

#include "GAS.h"
#include "GasNodes.h"

TYPE_DEF_CHILD(Component, GasPlayer);

GasPlayer::GasPlayer(Actor* owner) : Component(owner)
{
    
}

void GasPlayer::Play(GAS* gas)
{
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

void GasPlayer::Stop()
{
    mGas = nullptr;

    //TODO: ok so, here's the expected behavior here:
    //TODO: if you purposely stop a fidget (like with StopFidget), all the cleanups and such should occur.
    //TODO: if you stop a fidget due to an error (tried to set a null fidget), it's supposed to IMMEDIATELY STOP (leaving any anims mid-play).
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

    /*
    // Handle any active "when no longer near" nodes.
    for(auto it = mActiveWhenNoLongerNearNodes.begin(); it != mActiveWhenNoLongerNearNodes.end();)
    {
        if((*it)->CheckCondition(this))
        {
            if(mActiveWhenNoLongerNearNodes.size() > 1)
            {
                std::swap(*it, mActiveWhenNoLongerNearNodes.back());
                mActiveWhenNoLongerNearNodes.pop_back();
            }
            else
            {
                mActiveWhenNoLongerNearNodes.pop_back();
                it = mActiveWhenNoLongerNearNodes.begin();
            }
        }
        else
        {
            ++it;
        }
    }
    */
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
    mNodeIndex++;
    mNodeIndex %= mGas->GetNodeCount();
    
    // Grab the node from the list.
    //std::cout << "Executing node " << mNodeIndex << std::endl;
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
