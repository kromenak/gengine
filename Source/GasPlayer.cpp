//
// GasPlayer.cpp
//
// Clark Kromenaker
//
#include "GasPlayer.h"
#include "GAS.h"

TYPE_DEF_CHILD(Component, GasPlayer);

GasPlayer::GasPlayer(Actor* owner) : Component(owner)
{
    
}

void GasPlayer::Update(float deltaTime)
{
    if(mGas == nullptr) { return; }
    
    // Decrement timer. When it gets to zero, we move onto the next node.
    if(mTimer >= 0.0f)
    {
        mTimer -= deltaTime;
        if(mTimer <= 0.0f)
        {
            ProcessNextNode();
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
    mNodeIndex++;
    mNodeIndex %= mGas->GetNodeCount();
    
    // Grab the node from the list.
    GasNode* node = mGas->GetNode(mNodeIndex);
    if(node == nullptr)
    {
        mTimer = 0.0f;
        return;
    }
    
    // Execute the node, which returns amount of time it will take to perform the node.
    int waitMilliseconds = node->Execute(this);
    
    // Update timer.
    mTimer = (float)waitMilliseconds / 1000.0f;
}
