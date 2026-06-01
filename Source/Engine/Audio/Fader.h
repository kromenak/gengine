//
// Clark Kromenaker
//
// Helpers for fading in/out audio.
//
#pragma once
#include <fmod.hpp>

// Handles fading in/out audio volume.
struct Fader
{
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;
    float fadeTo = 0.0f;
    float fadeFrom = 0.0f;
    FMOD::ChannelControl* channelControl = nullptr;

    Fader() = default;
    Fader(FMOD::ChannelControl* cc) : channelControl(cc) { }

    bool Update(float deltaTime);
    void SetFade(float fadeTime, float targetVolume, float startVolume = -1.0f);
};

// Handles crossfading two sets of playing audio.
struct Crossfader
{
    // The channel group. The fade groups are inputs to this, and this outputs to master.
    // This allows modifying volume in a consistent way regardless of the fade state of the fade groups.
    FMOD::ChannelGroup* channelGroup = nullptr;

    // Two channel groups that feed into the above channel group.
    // These are swapped between to enable crossfading of audio tracks.
    FMOD::ChannelGroup* faderChannelGroups[2] { nullptr, nullptr };
    Fader faders[2];
    int fadeIndex = 0;

    bool Init(FMOD::System* system, const char* name);
    void Update(float deltaTime);
    void Swap();
    FMOD::ChannelGroup* GetActive() const { return faderChannelGroups[fadeIndex]; }
};