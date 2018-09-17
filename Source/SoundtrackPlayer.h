//
// SoundtrackPlayer.h
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

class SoundtrackPlayer : public Component
{
    TYPE_DECL_CHILD();
public:
    SoundtrackPlayer(Actor* owner);
    
    void Update(float deltaTime) override;
    
    void Play(Soundtrack* soundtrack);
    
private:
    // The soundtrack currently being played.
    Soundtrack* mSoundtrack = nullptr;
    
    // Copy of nodes from the soundtrack.
    // We need to create a copy so we can update repeat values.
    std::vector<SoundtrackNode*> mSoundtrackNodes;
    
    // Current index within soundtrack nodes that we are processing.
    int mCurrentNodeIndex = 0;
    
    // A timer we'll use to determine when we should move on from current node.
    float mTimer = 0.0f;
    
    void ProcessNextNode();
};
