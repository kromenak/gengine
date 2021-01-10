//
//  AudioManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#pragma once
#include <vector>

#include <fmod.hpp>
#include <fmod_errors.h>

class Audio;
class Soundtrack;

class Vector3;
class Quaternion;

enum class AudioType
{
    SFX,
    VO,
    Ambient
};

struct AudioState
{
    std::vector<FMOD::Channel*> channels;
};

class AudioManager
{
public:
    // If calling code doesn't provide specific min/max 3D dists, we'll use these.
    static constexpr float kDefault3DMinDist = 0.0f;
    static constexpr float kDefault3DMaxDist = 1000.0f;
    
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    void UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up);
    
    void PlaySFX(Audio* audio);
    void PlaySFX3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    void PlayVO(Audio* audio);
    void PlayVO3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    void PlayAmbient(Audio* audio, int fadeInMs);
    void PlayAmbient3D(Audio* audio, const Vector3& position, float minDist = kDefault3DMinDist, float maxDist = kDefault3DMaxDist);
    
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    void SetVolume(AudioType audioType, float volume);
    float GetVolume(AudioType audioType) const;
    
    AudioState SaveAudioState();
    void RestoreAudioState(const AudioState& audioState);
    
private:
    // Underlying FMOD system - portal to audio greatness.
    FMOD::System* mSystem = nullptr;
    
    // Channel groups for different types of audio.
    // Allows changing volume based on audio type.
    FMOD::ChannelGroup* mSFXChannelGroup = nullptr;
    FMOD::ChannelGroup* mVOChannelGroup = nullptr;
    FMOD::ChannelGroup* mAmbientChannelGroup = nullptr;
    
    // Master channel group allows modifying master volume.
    // All other channel groups are children of this group.
    FMOD::ChannelGroup* mMasterChannelGroup = nullptr;
    
    // Channels that are currently playing.
    std::vector<FMOD::Channel*> mPlayingChannels;
    
    FMOD::ChannelGroup* GetChannelGroupForAudioType(AudioType audioType) const;
    
    FMOD::Channel* CreateAndPlaySound(const char* buffer, int bufferLength, AudioType audioType, bool is3D = false);
    FMOD::Channel* CreateAndPlaySound3D(const char* buffer, int bufferLength, AudioType audioType, const Vector3& position, float minDist, float maxDist);
};
