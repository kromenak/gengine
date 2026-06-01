//
// Clark Kromenaker
//
// Handles playback of all types of audio.
//
#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include <fmod.hpp>

#include "Fader.h"
#include "PlayingSoundHandle.h"
#include "Vector3.h"

class Audio;

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

    void Pause();
    void Resume();

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
    static constexpr float kSFXVolumeMultiplier = 1.0f;
    static constexpr float kVOVolumeMultiplier = 1.0f;
    static constexpr float kAmbientVolumeMultiplier = 1.0f;
    static constexpr float kMusicVolumeMultiplier = 0.6f;

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

    float GetVolumeMultiplierForAudioType(AudioType audioType) const;
};

extern AudioManager gAudioManager;