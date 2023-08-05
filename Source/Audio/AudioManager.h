//
// Clark Kromenaker
//
// Handles playback of all types of audio.
//
#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include <fmod.hpp>
#include <fmod_errors.h>

#include "Vector3.h"

class Audio;
class Vector3;

// Handle to a playing sound. Returned by audio manager to represent a playing sound.
// After sound stops/finishes, the handle is no longer valid. But it can still be passed around, used, and stored just fine.
// Calling functions on an invalid handle will have no effect.
class PlayingSoundHandle
{
public:
    PlayingSoundHandle() = default;
    PlayingSoundHandle(FMOD::Channel* channel, FMOD::Sound* sound);

    void Stop(float fadeOutTime = 0.0f);
    void Pause();
    void Resume();
    bool IsPlaying() const;
    
    void SetVolume(float volume);
    void SetPosition(const Vector3& position);

private:
    friend class AudioManager; // Allow audio manager to access internals.

    // An FMOD "channel" represents a single playing sound. Channel* is returned by PlaySound.
    // Once returned, this pointer is always valid, even if the sound stops playing or the channel is reused.
    // In one of those scenarios, calls to channel functions start to return FMOD_ERR_INVALID_HANDLE or similar result codes.
    FMOD::Channel* channel = nullptr;

    // Sound data being played.
    FMOD::Sound* sound = nullptr;
    
    // Callback to execute when sound finishes playing (either naturally or via stop).
    std::function<void()> mFinishCallback;

    // The frame this sound started on.
    uint32_t mStartFrame = 0;
};

struct AudioSaveState
{
    std::vector<PlayingSoundHandle> playingSounds;
};

enum class AudioType
{
    SFX,        // Audio plays on SFX channels
    VO,         // Audio plays on VO channels
    Ambient,    // Audio plays on Ambient channels
    Music       // Audio plays on Music channels
};

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

struct PlayAudioParams
{
    // The audio file to play.
    Audio* audio = nullptr;

    // The type of audio. This indirectly controls volume based on player preferences.
    AudioType audioType = AudioType::SFX;

    // The amount of time to fade in the sound. Zero means no fade-in.
    float fadeInTime = 0.0f;

    // The volume to play the audio at, from 0.0f (0%) to 1.0f (100%).
    float volume = 1.0f;

    // Number of times to loop.
    // -1 = loop forever, 0 = no loop, 1+ = loop X many times
    int loopCount = 0;

    // Called when the sound finishes playing.
    std::function<void()> finishCallback = nullptr;

    // Should we play this as a 3D sound?
    bool is3d = false;

    // For 3D positional audio, the point the audio plays from.
    Vector3 position;

    // For 3D positional audio, the distances from which the sound is audible.
    // Sound will be loudest at min dist, quietest at max dist.
    // Negative values will use the defaults.
    float minDist = -1.0f;
    float maxDist = -1.0f;
};

class AudioManager
{
public:
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up);

    PlayingSoundHandle Play(const PlayAudioParams& params);
    PlayingSoundHandle PlaySFX(Audio* audio, std::function<void()> finishCallback = nullptr);

    void Stop(Audio* audio);
    void Stop(PlayingSoundHandle& soundHandle, float fadeOutTime = 0.0f);
    void StopAll();
    void StopOnOrAfterFrame(uint32_t frame);

    void ReleaseAudioData(Audio* audio);

    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    void SetVolume(AudioType audioType, float volume);
    float GetVolume(AudioType audioType) const;
    
    void SetMuted(bool mute);
    bool GetMuted();
    
    void SetMuted(AudioType audioType, bool mute);
    bool GetMuted(AudioType audioType);
    
    void SaveAudioState(bool sfx, bool vo, bool ambient, AudioSaveState& saveState);
    void RestoreAudioState(AudioSaveState& audioState);
    
private:
    // Underlying FMOD system - portal to audio greatness.
    FMOD::System* mSystem = nullptr;

    // Master channel group is the final output destination for all other channel groups.
    // Changes to this group affect all other groups.
    FMOD::ChannelGroup* mMasterChannelGroup = nullptr;

    // Channel group for SFX. Outputs to master channel group.
    FMOD::ChannelGroup* mSFXChannelGroup = nullptr;

    // Channel group for VO. Outputs to master channel group.
    FMOD::ChannelGroup* mVOChannelGroup = nullptr;

    // Channel groups for ambient and music. Both output to master channel group.
    FMOD::ChannelGroup* mAmbientChannelGroup = nullptr;
    FMOD::ChannelGroup* mMusicChannelGroup = nullptr;

    // Any individual sound can be faded in/out. If needed, one of these faders will be used for that purpose.
    std::vector<Fader> mFaders;

    // Volume multipliers for each audio type. This changes the range of possible volumes for a particular type of audio.
    // For example, if 0.8 is used, it means that "max volume" for that audio type is actually 80% of the "true max".
    // This is just helpful for achieving a good sounding mix. In particular, music tends to overpower SFX/VO, so compensate for that.
    const float kSFXVolumeMultiplier = 0.6f;
    const float kVOVolumeMultiplier = 1.0f;
    const float kAmbientVolumeMultiplier = 1.0f;
    const float kMusicVolumeMultiplier = 0.6f;

    // Default min/max distances for 3D sound volumes.
    // If calling code doesn't provide specific min/max 3D dists, we'll use these.
    float mDefault3DMinDist = 200.0f;
    float mDefault3DMaxDist = 2000.0f;
    
    // Sounds that are currently playing.
    std::vector<PlayingSoundHandle> mPlayingSounds;

    // Mapping from Audio assets to FMOD's internal sound instances.
    // This mapping stops us from creating multiple FMOD sounds for a single Audio (essentially a memory leak).
    std::unordered_map<Audio*, FMOD::Sound*> mFmodAudioData;

    // When an Audio asset is deleted, we want to release the underlying FMOD sound data too.
    // However, if the sound is still playing, we don't want to release until it ends or stops.
    std::vector<FMOD::Sound*> mWaitingToRelease;
    
    FMOD::Sound* CreateSound(Audio* audio, AudioType audioType, bool is3D, bool isLooping);
    void DestroySound(FMOD::Sound* sound);

    FMOD::ChannelGroup* GetChannelGroupForAudioType(AudioType audioType) const;
    FMOD::Channel* CreateChannel(FMOD::Sound* sound, FMOD::ChannelGroup* channelGroup);
};

extern AudioManager gAudioManager;