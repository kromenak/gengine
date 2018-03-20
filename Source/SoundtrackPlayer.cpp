//
// SoundtrackPlayer.cpp
//
// Clark Kromenaker
//
#include "SoundtrackPlayer.h"

SoundtrackPlayer::SoundtrackPlayer(Actor* owner) : Component(owner)
{
    
}

void SoundtrackPlayer::Update(float deltaTime)
{
    if(mSoundtrack == nullptr) { return; }
    if(mSoundtrackNodes.size() == 0) { return; }
    
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

void SoundtrackPlayer::Play(Soundtrack *soundtrack)
{
    //TODO: If an existing sound track exists, clean it up.
    
    // Save reference to new soundtrack.
    mSoundtrack = soundtrack;
    if(mSoundtrack == nullptr) { return; }
    
    // Get a copy of the soundtrack's nodes.
    mSoundtrackNodes = mSoundtrack->GetNodesCopy();
    
    // Make sure all nodes are reset.
    for(auto& node : mSoundtrackNodes)
    {
        node->Reset();
    }
    
    // Process the first node in the list of nodes.
    mCurrentNodeIndex = -1;
    ProcessNextNode();
}

void SoundtrackPlayer::ProcessNextNode()
{
    // If no nodes, can't do anything!
    if(mSoundtrackNodes.size() == 0)
    {
        mTimer = 0.0f;
        return;
    }
    
    // First off, if current node is looping...just keep doing it! It never stops!
    if(mCurrentNodeIndex >= 0 && mSoundtrackNodes[mCurrentNodeIndex]->IsLooping())
    {
        mTimer = mSoundtrackNodes[mCurrentNodeIndex]->Execute();
        return;
    }
    
    // First, update the node index and loop if necessary.
    mCurrentNodeIndex++;
    mCurrentNodeIndex %= mSoundtrackNodes.size();
    
    // Grab the node from the list.
    SoundtrackNode* node = mSoundtrackNodes[mCurrentNodeIndex];
    int waitMilliseconds = node->Execute();
    mTimer = (float)waitMilliseconds / 1000.0f;
}
