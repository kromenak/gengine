#include "SoundtrackPlayer.h"

PlayingSoundtrack::PlayingSoundtrack(Soundtrack* soundtrack) :
    mSoundtrack(soundtrack)
{
    // Make sure all nodes are reset.
    for(auto& node : mSoundtrack->GetNodes())
    {
        mExecutionCounts.push_back(0);
    }

    // Get things rolling.
    ProcessNextNode();
}

void PlayingSoundtrack::Update(float deltaTime)
{
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

void PlayingSoundtrack::ProcessNextNode()
{
    // If no nodes, can't do anything!
    auto& nodes = mSoundtrack->GetNodes();
    if(nodes.size() == 0)
    {
        mTimer = 0.0f;
        return;
    }

    // First off, if current node is looping...just keep doing it! It never stops!
    if(mCurrentNodeIndex >= 0 && nodes[mCurrentNodeIndex]->IsLooping())
    {
        mTimer = nodes[mCurrentNodeIndex]->Execute(mSoundtrack->GetSoundType());
        return;
    }

    // First, update the node index and loop if necessary.
    mCurrentNodeIndex++;
    mCurrentNodeIndex %= nodes.size();

    // Grab the node from the list.
    SoundtrackNode* node = nodes[mCurrentNodeIndex];

    // If this node has a repeat limit, check whether it has been surpassed.
    // If so, just move on to the next node right away.
    if(node->repeat > 0 && node->repeat - mExecutionCounts[mCurrentNodeIndex] <= 0)
    {
        mTimer = 0.0f;
        return;
    }

    // Ok, execute the thing.
    mExecutionCounts[mCurrentNodeIndex]++;
    int waitMilliseconds = node->Execute(mSoundtrack->GetSoundType());
    mTimer = (float)waitMilliseconds / 1000.0f;
}

TYPE_DEF_CHILD(Component, SoundtrackPlayer);

SoundtrackPlayer::SoundtrackPlayer(Actor* owner) : Component(owner)
{
    
}

void SoundtrackPlayer::Play(Soundtrack* soundtrack)
{
    if(soundtrack == nullptr) { return; }

    // See if already playing. If so, don't do anything!
    for(auto& playing : mPlaying)
    {
        if(playing.mSoundtrack == soundtrack)
        {
            return;
        }
    }

    // Ok, we are going to play this thing.
    mPlaying.emplace_back(soundtrack);
}

void SoundtrackPlayer::Stop(Soundtrack* soundtrack)
{
    if(soundtrack == nullptr) { return; }

    // Find and remove.
    for(auto it = mPlaying.begin(); it != mPlaying.end(); ++it)
    {
        if(it->mSoundtrack == soundtrack)
        {
            // Stopping a soundtrack stops any additional nodes from executing.
            // But it DOES NOT stop currently playing audio.
            mPlaying.erase(it);
            return;
        }
    }
}

void SoundtrackPlayer::StopAll()
{
    // This will stop any further nodes from executing.
    // Stopping a soundtrack DOES NOT stop currently playing audio.
    mPlaying.clear();
}

void SoundtrackPlayer::OnUpdate(float deltaTime)
{
    for(auto& playing : mPlaying)
    {
        playing.Update(deltaTime);
    }
}
