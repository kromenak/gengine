#include "SoundtrackPlayer.h"

#include "ActionManager.h"
#include "GKActor.h"
#include "StringUtil.h"

PlayingSoundtrack::PlayingSoundtrack(Soundtrack* soundtrack) :
    mSoundtrack(soundtrack)
{
    
}

void PlayingSoundtrack::Play()
{
    // Make sure all nodes are reset.
    mExecutionCounts.clear();
    mExecutionCounts.resize(mSoundtrack->GetNodes().size());

    // Get things rolling.
    ProcessNextNode();
}

void PlayingSoundtrack::Stop()
{
    // If a sound is currently playing as part of this soundtrack, we need to stop it in the desired fashion.
    if(mSoundtrackNodeResults.stopMethod == StopMethod::Immediate)
    {
        mSoundtrackNodeResults.soundHandle.Stop();
    }
    else if(mSoundtrackNodeResults.stopMethod == StopMethod::FadeOut)
    {
        mSoundtrackNodeResults.soundHandle.Stop(mSoundtrackNodeResults.fadeOutTimeMs * 0.001f);
    }
}

void PlayingSoundtrack::Update(float deltaTime)
{
    // If the previous node designated a follow object, keep the playing sound's 3D position updated to "follow" that object.
    if(mSoundtrackNodeResults.followObj != nullptr)
    {
        mSoundtrackNodeResults.soundHandle.SetPosition(mSoundtrackNodeResults.followObj->GetAudioPosition());
    }

    // If the current node is looping, we can early out - there is NO way to advance past a looping node.
    // The actual looping behavior is controlled by the audio system.
    if(mCurrentNodeIndex >= 0 && mSoundtrack->GetNodes()[mCurrentNodeIndex]->IsLooping())
    {
        return;
    }

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

    // Update the node index and loop if necessary.
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
    int waitMilliseconds = node->Execute(mSoundtrack, mSoundtrackNodeResults);
    mTimer = (float)waitMilliseconds / 1000.0f;
}

TYPE_DEF_CHILD(Component, SoundtrackPlayer);

SoundtrackPlayer::SoundtrackPlayer(Actor* owner) : Component(owner)
{
    
}

SoundtrackPlayer::~SoundtrackPlayer()
{
    StopAll();
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
    mPlaying.back().Play();
}

void SoundtrackPlayer::Stop(Soundtrack* soundtrack)
{
    if(soundtrack == nullptr) { return; }

    // Find and remove.
    for(auto it = mPlaying.begin(); it != mPlaying.end(); ++it)
    {
        if(it->mSoundtrack == soundtrack)
        {
            // Stop the soundtrack.
            // How the audio stops depends on the "Stop Method" of the current soundtrack node.
            it->Stop();

            // Erase from list.
            mPlaying.erase(it);
            return;
        }
    }
}

void SoundtrackPlayer::Stop(const std::string& soundtrackName)
{
    // Find and remove.
    for(auto it = mPlaying.begin(); it != mPlaying.end(); ++it)
    {
        if(StringUtil::EqualsIgnoreCase(it->mSoundtrack->GetNameNoExtension(), soundtrackName))
        {
            // Stop the soundtrack.
            // How the audio stops depends on the "Stop Method" of the current soundtrack node.
            it->Stop();

            // Erase from list.
            mPlaying.erase(it);
            return;
        }
    }
}

void SoundtrackPlayer::StopAll()
{
    // Stop all playing soundtracks.
    for(PlayingSoundtrack& playing : mPlaying)
    {
        playing.Stop();
    }
    mPlaying.clear();
}

void SoundtrackPlayer::OnUpdate(float deltaTime)
{
    if(gActionManager.IsSkippingCurrentAction()) { return; }

    for(PlayingSoundtrack& playing : mPlaying)
    {
        playing.Update(deltaTime);
    }
}
