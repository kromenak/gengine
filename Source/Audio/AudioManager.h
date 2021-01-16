//
// AudioManager.h
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

struct AudioState
{
    std::vector<FMOD::Channel*> channels;
};

// A "handle" for a single playing sound. Basically a wrapper around a FMOD channel.
// Returned by audio manager functions that start sound playback to allow the caller to query and modify the playing sound.
// After the sound stops (either plays all the way or is stopped prematurely), the handle is no longer valid.
// Even if the handle is not valid, it can still be passed around and used, but just will no longer have any effect.
class PlayingSoundHandle
{
public:
    PlayingSoundHandle() = default;
    PlayingSoundHandle(FMOD::Channel* channel);
    
    void SetVolume(float volume);
    bool IsPlaying() const;
    
private:
    // Allow audio manager to access channel directly, but not others.
    friend class AudioManager;
    
    // An FMOD "channel" represents a single playing sound. Channel* is returned by PlaySound.
    // Once returned, this pointer is always valid, even if the sound stops playing or the channel is reused.
    // In one of those scenarios, calls to channel functions start to return FMOD_ERR_INVALID_HANDLE or similar result codes.
    FMOD::Channel* channel = nullptr;
};

enum class AudioType
{
    SFX,
    VO,
    Ambient
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
    
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    void SetVolume(AudioType audioType, float volume);
    float GetVolume(AudioType audioType) const;
    
    AudioState SaveAudioState();
    void RestoreAudioState(const AudioState& audioState);
    
private:
    // Underlying FMOD system - portal to audio greatness.
    FMOD::System* mSystem = nullptr;
        
    // Channel groups for different types of audio. Children of "master" group.
    // Allows changing volume based on audio type.
    FMOD::ChannelGroup* mSFXChannelGroup = nullptr;
    FMOD::ChannelGroup* mVOChannelGroup = nullptr;
    FMOD::ChannelGroup* mAmbientChannelGroup = nullptr;
    
    // Master channel group allows modifying master volume.
    // All other channel groups are children of this group.
    FMOD::ChannelGroup* mMasterChannelGroup = nullptr;
    
    // Channel group just for fading in/out ambient audio. Child of "ambient" channel group.
    // Don't want to modify sound or ambient group directly (they can be modified by others). This is JUST for fade in/out.
    FMOD::ChannelGroup* mAmbientFadeChannelGroup = nullptr;
    
    //
    float mAmbientFadeDuration = 0.0f;
    float mAmbientFadeTimer = 0.0f;
    float mAmbientFadeTo = 0.0f;
    float mAmbientFadeFrom = 0.0f;
    
    // Volume multipliers for each audio type. This changes the range of possible volumes for a particular type of audio.
    // For example, if 0.8 is used, it means that "max volume" for that audio type is actually 80% of the "true max".
    // This is just helpful for achieving a good sounding mix. In particular, music tends to overpower SFX/VO, so compensate for that.
    const float kSFXVolumeMultiplier = 1.0f;
    const float kVOVolumeMultiplier = 1.0f;
    const float kAmbientVolumeMultiplier = 0.75f;
    
    // Sounds that are currently playing.
    std::vector<PlayingSoundHandle> mPlayingSounds;
    
    FMOD::ChannelGroup* GetChannelGroupForAudioType(AudioType audioType) const;
    
    PlayingSoundHandle CreateAndPlaySound(const char* buffer, int bufferLength, AudioType audioType, bool is3D = false);
    PlayingSoundHandle CreateAndPlaySound3D(const char* buffer, int bufferLength, AudioType audioType, const Vector3& position, float minDist, float maxDist);
    
    void AmbientFade(float fadeTime, float targetVolume, float startVolume = -1.0f);
    
    void AmbientFadeIn(float fadeInTime);
    void AmbientFadeOut(float fadeOutTime);
};
