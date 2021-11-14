//
// Clark Kromenaker
//
// Handles playback of all types of audio.
//
#pragma once
#include <vector>

#include <fmod.hpp>
#include <fmod_errors.h>

class Audio;
class Vector3;

// Handle to a playing sound. Returned by audio manager to represent a playing sound.
// After sound stops/finishes, the handle is no longer valid. But it can still be passed around, used, and stored just fine.
// Calling functions on an invalid handle will have no effect.
class PlayingSoundHandle
{
public:
    PlayingSoundHandle() = default;
    PlayingSoundHandle(FMOD::Channel* channel);

    void Stop();
    void Pause();
    void Resume();
    
    void SetVolume(float volume);
    bool IsPlaying() const;
    
private:
    friend class AudioManager; // Allow audio manager to access channel directly.

    // The Audio asset that triggered this sound to play.
    Audio* audio = nullptr;
    
    // An FMOD "channel" represents a single playing sound. Channel* is returned by PlaySound.
    // Once returned, this pointer is always valid, even if the sound stops playing or the channel is reused.
    // In one of those scenarios, calls to channel functions start to return FMOD_ERR_INVALID_HANDLE or similar result codes.
    FMOD::Channel* channel = nullptr;
};

struct AudioSaveState
{
    std::vector<PlayingSoundHandle> playingSounds;
};

enum class AudioType
{
    SFX,        // Audio plays on SFX channel
    VO,         // Audio plays on VO channel
    Ambient,    // Audio plays on Ambient channels and crossfades between scenes
    Music       // Audio plays on Music channels and crossfades between scenes
};

// Handles fading in/out audio volume.
struct Fader
{
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;
    float fadeTo = 0.0f;
    float fadeFrom = 0.0f;
    FMOD::ChannelGroup* channelGroup = nullptr;

    void Update(float deltaTime);
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
    Fader faders[2];
    int fadeIndex = 0;

    bool Init(FMOD::System* system, const char* name);
    void Update(float deltaTime);
    void Swap();
    FMOD::ChannelGroup* GetActive() const { return faders[fadeIndex].channelGroup; }
};

class AudioManager
{
public:
    // If calling code doesn't provide specific min/max 3D dists, we'll use these.
    //TODO: values are pulled from GAME.CFG; maybe we should read in that file.
    static constexpr float kDefault3DMinDist = 200.0f;
    static constexpr float kDefault3DMaxDist = 2000.0f;
    
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up);
    
    PlayingSoundHandle PlaySFX(Audio* audio);
    PlayingSoundHandle PlaySFX3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    PlayingSoundHandle PlayVO(Audio* audio);
    PlayingSoundHandle PlayVO3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    PlayingSoundHandle PlayAmbient(Audio* audio, float fadeInTime = 0.0f);
    PlayingSoundHandle PlayAmbient3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);

    PlayingSoundHandle PlayMusic(Audio* audio, float fadeInTime = 0.0f);

    void Stop(Audio* audio);
    void StopAll();

    void SwapAmbient();

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

    // Both ambient audio and music can crossfade between scenes, so these crossfaders help with that.
    // Internally, they also consist of several FMOD channel groups used to control volume. Both output to master channel group ultimately.
    Crossfader mAmbientCrossfade;
    Crossfader mMusicCrossfade;
    
    // Volume multipliers for each audio type. This changes the range of possible volumes for a particular type of audio.
    // For example, if 0.8 is used, it means that "max volume" for that audio type is actually 80% of the "true max".
    // This is just helpful for achieving a good sounding mix. In particular, music tends to overpower SFX/VO, so compensate for that.
    const float kSFXVolumeMultiplier = 1.0f;
    const float kVOVolumeMultiplier = 1.0f;
    const float kAmbientVolumeMultiplier = 1.0f;
    const float kMusicVolumeMultiplier = 0.75f;
    
    // Sounds that are currently playing.
    std::vector<PlayingSoundHandle> mPlayingSounds;
    
    FMOD::ChannelGroup* GetChannelGroupForAudioType(AudioType audioType, bool forVolume) const;
    
    PlayingSoundHandle CreateAndPlaySound(Audio* audio, AudioType audioType, bool is3D = false);
    PlayingSoundHandle CreateAndPlaySound3D(Audio* audio, AudioType audioType, const Vector3& position, float minDist, float maxDist);
};
