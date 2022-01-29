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

struct PlayingSoundtrack
{
public:
    // The soundtrack currently being played.
    Soundtrack* mSoundtrack = nullptr;

    PlayingSoundtrack(Soundtrack* soundtrack);

    void Play();
    void Stop();

    void Update(float deltaTime);

private:
    void ProcessNextNode();

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
    TYPE_DECL_CHILD();
public:
    SoundtrackPlayer(Actor* owner);
    ~SoundtrackPlayer();
    
    void Play(Soundtrack* soundtrack);
    void Stop(Soundtrack* soundtrack);
    void StopAll();
	
protected:
	void OnUpdate(float deltaTime) override;
    
private:
    std::vector<PlayingSoundtrack> mPlaying;
};
