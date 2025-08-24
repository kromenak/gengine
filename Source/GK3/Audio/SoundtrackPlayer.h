//
// Clark Kromenaker
//
// A component that can play a Soundtrack asset.
//
// A Soundtrack asset is a fairly complicated node-based list structure
// that is used to create a complicated musical composition from smaller
// chunks of audio.
//
// The logic for playing a soundtrack asset is fairly specialized,
// so a custom component for the purpose seems reasonable.
//
#pragma once
#include "Component.h"

#include <vector>

#include "Soundtrack.h"

class PersistState;

struct PlayingSoundtrack
{
public:
    // The soundtrack currently being played.
    Soundtrack* mSoundtrack = nullptr;

    PlayingSoundtrack(Soundtrack* soundtrack, bool nonLooping);

    void Play();
    void Stop(bool force = false);

    void Update(float deltaTime);

    bool AllNodesHitRepeatLimit() const;

private:
    void ProcessNextNode();

    // If true, this soundtrack doesn't loop.
    bool mNonLooping = false;

    // The results from executing the most recent soundtrack node.
    SoundtrackNodeResults mSoundtrackNodeResults;

    // Some nodes have repeat limits, so we need to know how many times we've executed each node.
    std::vector<int> mExecutionCounts;

    // Current index within soundtrack nodes that we are processing.
    int mCurrentNodeIndex = -1;

    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;
};

class SoundtrackPlayer : public Component
{
    TYPEINFO_SUB(SoundtrackPlayer, Component);
public:
    SoundtrackPlayer(Actor* owner);
    ~SoundtrackPlayer();

    void Play(Soundtrack* soundtrack, bool nonLooping = false);
    void Stop(Soundtrack* soundtrack, bool force = false);
    void Stop(const std::string& soundtrackName, bool force = false);
    void StopAll(bool force = false);

    void OnPersist(PersistState& ps);

protected:
    void OnUpdate(float deltaTime) override;

private:
    std::vector<PlayingSoundtrack> mPlaying;
};
