#include "AudioManager.h"

#include <cstring>
#include <iostream>

#include <fmod_errors.h>

#include "AssetManager.h"
#include "Audio.h"
#include "GEngine.h"
#include "GMath.h"
#include "Profiler.h"
#include "ReportManager.h"
#include "SaveManager.h"

AudioManager gAudioManager;

bool AudioManager::Initialize()
{
    TIMER_SCOPED("AudioManager::Initialize");

    // Create the FMOD system.
    FMOD_RESULT result = FMOD::System_Create(&mSystem);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    // Retrieve the FMOD version.
    unsigned int version;
    result = mSystem->getVersion(&version);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to get FMOD version: %s", FMOD_ErrorString(result));
        return false;
    }

    // Verify that the FMOD library version matches the header version.
    if(version < FMOD_VERSION)
    {
        LOG_ERROR("FMOD library version %u doesn't match header version %u.", version, FMOD_VERSION);
        return false;
    }

    // If desired, set DSP buffer size to something other than the default of 1024.
    // EXPERIMENTAL: one user reported crackling audio, so I'm curious if this helps resolve it.
    Config* userConfig = gAssetManager.LoadAsset<Config>("GK3.ini");
    if(userConfig != nullptr)
    {
        int dspBufferSize = userConfig->GetInt("Audio", "DSP Buffer Size", 1024);
        if(dspBufferSize != 1024)
        {
            mSystem->setDSPBufferSize(dspBufferSize, 4);
        }
    }

    // Initialize the FMOD system.
    result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to init FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    // After some trial/error, it seems like GK3's rolloff is quicker than FMOD's default.
    // Using a value of 2.0f for "rolloffScale" causes volume to diminish a bit more quickly as you move away from an object.
    result = mSystem->set3DSettings(1.0f, 1.0f, 1.0f);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to set FMOD 3D settings: %s", FMOD_ErrorString(result));
        return false;
    }

    // Create SFX channel group.
    result = mSystem->createChannelGroup("SFX", &mSFXChannelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD SFX channel group: %s", FMOD_ErrorString(result));
        return false;
    }

    // Create VO channel group.
    result = mSystem->createChannelGroup("VO", &mVOChannelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD VO channel group: %s", FMOD_ErrorString(result));
        return false;
    }

    // Create ambient channel group.
    result = mSystem->createChannelGroup("Ambient", &mAmbientChannelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD Ambient channel group: %s", FMOD_ErrorString(result));
        return false;
    }

    // Create music channel group.
    result = mSystem->createChannelGroup("Music", &mMusicChannelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD Music channel group: %s", FMOD_ErrorString(result));
        return false;
    }

    // Get master channel group.
    result = mSystem->getMasterChannelGroup(&mMasterChannelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to get FMOD master channel group: %s", FMOD_ErrorString(result));
        return false;
    }

    // Set mute based on audio prefs.
    Config* prefs = gSaveManager.GetPrefs();
    bool globalEnabled = prefs->GetBool(PREFS_SOUND, PREFS_AUDIO_ENABLED, true);
    SetMuted(!globalEnabled);

    bool sfxEnabled = prefs->GetBool(PREFS_SOUND, PREFS_SFX_ENABLED, true);
    SetMuted(AudioType::SFX, !sfxEnabled);

    bool voEnabled = prefs->GetBool(PREFS_SOUND, PREFS_VO_ENABLED, true);
    SetMuted(AudioType::VO, !voEnabled);

    bool ambientEnabled = prefs->GetBool(PREFS_SOUND, PREFS_AMBIENT_ENABLED, true);
    SetMuted(AudioType::Ambient, !ambientEnabled);

    bool musicEnabled = prefs->GetBool(PREFS_SOUND, PREFS_MUSIC_ENABLED, true);
    SetMuted(AudioType::Music, !musicEnabled);

    // Set volumes for each audio type based on audio prefs.
    float globalVolume = prefs->GetInt(PREFS_SOUND, PREFS_AUDIO_VOLUME, 100) / 100.0f;
    SetMasterVolume(globalVolume);

    float sfxVolume = prefs->GetInt(PREFS_SOUND, PREFS_SFX_VOLUME, 100) / 100.0f;
    SetVolume(AudioType::SFX, sfxVolume);

    float voVolume = prefs->GetInt(PREFS_SOUND, PREFS_VO_VOLUME, 100) / 100.0f;
    SetVolume(AudioType::VO, voVolume);

    float ambientVolume = prefs->GetInt(PREFS_SOUND, PREFS_AMBIENT_VOLUME, 100) / 100.0f;
    SetVolume(AudioType::Ambient, ambientVolume);

    float musicVolume = prefs->GetInt(PREFS_SOUND, PREFS_MUSIC_VOLUME, 100) / 100.0f;
    SetVolume(AudioType::Music, musicVolume);

    // Grab defaults from GAME.CFG.
    Config* gameConfig = gAssetManager.LoadAsset<Config>("GAME.CFG");
    if(gameConfig != nullptr)
    {
        mDefault3DMinDist = gameConfig->GetFloat("Sound", "Default Sound Min Distance", mDefault3DMinDist);
        mDefault3DMaxDist = gameConfig->GetFloat("Sound", "Default Sound Max Distance", mDefault3DMaxDist);
    }

    // We initialized audio successfully!
    return true;
}

void AudioManager::Shutdown()
{
    // Close and release FMOD system.
    mSystem->close();
    mSystem->release();
    mSystem = nullptr;
}

void AudioManager::Pause()
{
    // Suspending the mixer ensures background threads sleep and don't use any CPU.
    mSystem->mixerSuspend();

    // Pause all playing sounds.
    for(PlayingSoundHandle& playingSound : mPlayingSounds)
    {
        playingSound.Pause();
    }
}

void AudioManager::Resume()
{
    // Resume the mixer.
    mSystem->mixerResume();

    // Resume all playing sounds.
    for(PlayingSoundHandle& playingSound : mPlayingSounds)
    {
        playingSound.Resume();
    }
}

void AudioManager::Update(float deltaTime)
{
    // Update FMOD system every frame.
    if(mSystem != nullptr)
    {
        mSystem->update();
    }

    // Update faders.
    for(size_t i = 0; i < mFaders.size(); ++i)
    {
        if(mFaders[i].Update(deltaTime))
        {
            std::swap(mFaders[i], mFaders[mFaders.size() - 1]);
            mFaders.pop_back();
            --i;
        }
    }

    // See if any playing channels are no longer playing.
    for(int i = mPlayingSounds.size() - 1; i >= 0; --i)
    {
        if(!mPlayingSounds[i].IsPlaying())
        {
            // Put dead sound on back of playing sounds vector.
            std::swap(mPlayingSounds[i], mPlayingSounds.back());

            // Save callback locally.
            auto callback = mPlayingSounds.back().mFinishCallback;

            // Remove from vector.
            mPlayingSounds.pop_back();

            // Execute callback if set.
            if(callback != nullptr)
            {
                callback();
            }
        }
    }

    // We just checked for stopped sounds, so all playing sounds are actually playing.
    // So, we can release any waiting FMOD::Sounds if no playing sound is using it.
    for(int i = mWaitingToRelease.size() - 1; i >= 0; --i)
    {
        bool stillPlaying = false;
        for(auto& playingSound : mPlayingSounds)
        {
            if(playingSound.sound == mWaitingToRelease[i])
            {
                stillPlaying = true;
                break;
            }
        }

        // Not playing? Release it finally!
        if(!stillPlaying)
        {
            DestroySound(mWaitingToRelease[i]);

            std::swap(mWaitingToRelease[i], mWaitingToRelease.back());
            mWaitingToRelease.pop_back();
        }
    }

    /*
    // For testing fade in/out behavior.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_M))
    {
        mAmbientFadeChannelGroups[mCurrentAmbientIndex].SetFade(1.0f, 1.0f);
    }
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_N))
    {
        mAmbientFadeChannelGroups[mCurrentAmbientIndex].SetFade(1.0f, 0.0f);
    }
    */
}

void AudioManager::UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up)
{
    FMOD_RESULT result = mSystem->set3DListenerAttributes(0, (const FMOD_VECTOR*)&position, (const FMOD_VECTOR*)&velocity,
                                                         (const FMOD_VECTOR*)&forward, (const FMOD_VECTOR*)&up);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to update FMOD 3D listener attributes: %s", FMOD_ErrorString(result));
    }
}

PlayingSoundHandle AudioManager::PlaySFX(Audio* audio, std::function<void()> finishCallback)
{
    PlayAudioParams params;
    params.audio = audio;
    params.audioType = AudioType::SFX;
    params.finishCallback = finishCallback;
    return Play(params);
}

PlayingSoundHandle AudioManager::Play(const PlayAudioParams& params)
{
    // We need a valid audio asset, for one.
    if(params.audio == nullptr) { return PlayingSoundHandle(); }

    // Create the sound from the audio buffer.
    FMOD::Sound* sound = CreateSound(params.audio, params.audioType, params.is3d, (params.loopCount < 0 || params.loopCount > 0));
    if(sound == nullptr)
    {
        LOG_WARNING("Failed to create FMOD sound.");
        return PlayingSoundHandle();
    }

    // Create the channel that will play the sound in the correct channel group.
    FMOD::Channel* channel = CreateChannel(sound, GetChannelGroupForAudioType(params.audioType));
    if(channel == nullptr)
    {
        LOG_WARNING("Failed to create FMOD channel.");
        return PlayingSoundHandle();
    }

    // Add to playing sounds.
    mPlayingSounds.emplace_back(channel, sound);

    // Store finish callback.
    mPlayingSounds.back().mFinishCallback = params.finishCallback;

    // If 3D, set positional and distance parameters.
    if(params.is3d)
    {
        // Sometimes, callers may pass negative values to mean "use default" for min/max dists.
        float minDist = params.minDist;
        float maxDist = params.maxDist;

        if(minDist < 0.0f) { minDist = mDefault3DMinDist; }
        if(maxDist < 0.0f) { maxDist = mDefault3DMaxDist; }

        // Make sure min/max dist are in valid ranges.
        if(maxDist < minDist) { maxDist = minDist; }
        if(minDist > maxDist) { minDist = maxDist; }

        // Set distance attributes.
        channel->set3DMinMaxDistance(minDist, maxDist);

        // Set position and no velocity.
        channel->set3DAttributes((const FMOD_VECTOR*)&params.position, nullptr);
    }

    // Set looping behavior for the channel.
    channel->setLoopCount(params.loopCount);
    if(params.loopCount < 0 || params.loopCount > 0)
    {
        // Add LOOP flag to channel. This allows looping to occur.
        // Note however that the SOUND must also have been loaded with the LOOP flag for *seamless* looping.
        FMOD_MODE mode;
        channel->getMode(&mode);
        mode |= FMOD_LOOP_NORMAL;
        channel->setMode(mode);
    }

    // Handle fade-in time if specified.
    float volume = Math::Clamp(params.volume, 0.0f, 1.0f);
    if(!Math::IsZero(params.fadeInTime))
    {
        // Force channel volume to start value to avoid any single frame wrong volumes.
        channel->setVolume(0.0f);

        // Create a fader, which will tick each frame and adjust volume as needed.
        mFaders.emplace_back(channel);
        mFaders.back().SetFade(params.fadeInTime, volume, 0.0f);
    }
    else
    {
        // If not fading in, just set the volume directly.
        channel->setVolume(volume);
    }

    // Ok, all attributes should be set - let's play the sound!
    channel->setPaused(false);

    // Return handle to caller.
    return mPlayingSounds.back();
}

void AudioManager::Stop(Audio* audio)
{
    if(audio != nullptr)
    {
        auto it = mFmodAudioData.find(audio);
        if(it != mFmodAudioData.end())
        {
            for(auto& sound : mPlayingSounds)
            {
                if(sound.sound == it->second)
                {
                    // After stopping, sound is removed from playing sounds during next update loop.
                    Stop(sound);
                    return;
                }
            }
        }
    }
}

void AudioManager::Stop(PlayingSoundHandle& soundHandle, float fadeOutTime)
{
    // Need a valid channel to stop the thing.
    if(soundHandle.channel == nullptr) { return; }

    // If no fade out is specified, just stop it right away - easy.
    if(Math::IsZero(fadeOutTime))
    {
        soundHandle.channel->stop();
        soundHandle.channel = nullptr;
        return;
    }

    // We have to fade out before stopping it - employ a fader for this.
    mFaders.emplace_back(soundHandle.channel);
    mFaders.back().SetFade(fadeOutTime, 0.0f);
}

void AudioManager::StopAll()
{
    for(auto& sound : mPlayingSounds)
    {
        sound.Stop();
    }
    mPlayingSounds.clear();
}

void AudioManager::StopOnOrAfterFrame(uint32_t frame)
{
    for(auto& sound : mPlayingSounds)
    {
        // Only interested in sounds that started on or after the given frame.
        if(sound.mStartFrame < frame) { continue; }

        // We'll ignore music and ambient sounds for now.
        // Since this function is primarily meant for stopping sounds during an action skip...
        FMOD::ChannelGroup* channelGroup;
        sound.channel->getChannelGroup(&channelGroup);
        if(channelGroup == mMusicChannelGroup || channelGroup == mAmbientChannelGroup) { continue; }

        // Stop this sound.
        sound.Stop();
    }
}

void AudioManager::ReleaseAudioData(Audio* audio)
{
    // Find whether FMOD sound data exists for this audio file.
    auto it = mFmodAudioData.find(audio);
    if(it != mFmodAudioData.end())
    {
        // See if the sound is still playing.
        bool stillPlaying = false;
        for(auto& playingSound : mPlayingSounds)
        {
            if(playingSound.sound == it->second)
            {
                stillPlaying = true;
                break;
            }
        }

        // If still playing, add it to list of data to release AFTER done playing.
        // Otherwise, we can release it right now!
        if(stillPlaying)
        {
            mWaitingToRelease.push_back(it->second);
        }
        else
        {
            DestroySound(it->second);
        }

        // Erase audio->sound mapping.
        mFmodAudioData.erase(it);
    }
}

void AudioManager::SetMasterVolume(float volume)
{
    // Set volume. FMOD expects a normalized 0-1 value.
    volume = Math::Clamp(volume, 0.0f, 1.0f);
    mMasterChannelGroup->setVolume(volume);
    gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_AUDIO_VOLUME, static_cast<int>(volume * 100));
}

float AudioManager::GetMasterVolume() const
{
    float volume = 0.0f;
    mMasterChannelGroup->getVolume(&volume);
    return volume;
}

void AudioManager::SetVolume(AudioType audioType, float volume)
{
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType);
    if(channelGroup == nullptr) { return; }

    // Clamp input volume to 0-1 range.
    // Do this before applying multiplier to avoid passing in like 5.0f and avoiding multiplier effects.
    volume = Math::Clamp(volume, 0.0f, 1.0f);

    // Save volume as a preference.
    switch(audioType)
    {
    default:
    case AudioType::SFX:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_SFX_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::VO:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_VO_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::Ambient:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_AMBIENT_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::Music:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_MUSIC_VOLUME, static_cast<int>(volume * 100));
        break;
    }

    // The volume passed in is the user's preference between 0% and 100% volume for this audio type.
    // But from a design perspective, we want to make certain sound types louder or softer, so internally we apply an additional multiplier.
    float multiplier = GetVolumeMultiplierForAudioType(audioType);
    float internalVolume = volume * multiplier;

    // Set volume. FMOD expects a normalized 0-1 value.
    channelGroup->setVolume(Math::Clamp(internalVolume, 0.0f, 1.0f));
}

float AudioManager::GetVolume(AudioType audioType) const
{
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType);
    if(channelGroup == nullptr) { return 0.0f; }

    // Kind of the opposite of "set volume" - get the volume in the FMOD system first.
    float internalVolume = 0.0f;
    channelGroup->getVolume(&internalVolume);

    // And then remove the internal multiplier to get the value for external usage.
    return internalVolume / GetVolumeMultiplierForAudioType(audioType);
}

void AudioManager::SetMuted(bool mute)
{
    mMasterChannelGroup->setMute(mute);
    gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_AUDIO_ENABLED, !mute);
}

bool AudioManager::GetMuted()
{
    bool mute = false;
    mMasterChannelGroup->getMute(&mute);
    return mute;
}

void AudioManager::SetMuted(AudioType audioType, bool mute)
{
    GetChannelGroupForAudioType(audioType)->setMute(mute);

    // Save prefs (grr, more switches).
    switch(audioType)
    {
    case AudioType::SFX:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_SFX_ENABLED, !mute);
        break;
    case AudioType::VO:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_VO_ENABLED, !mute);
        break;
    case AudioType::Ambient:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_AMBIENT_ENABLED, !mute);
        break;
    case AudioType::Music:
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_MUSIC_ENABLED, !mute);
        break;
    }
}

bool AudioManager::GetMuted(AudioType audioType)
{
    bool mute = false;
    GetChannelGroupForAudioType(audioType)->getMute(&mute);
    return mute;
}

void AudioManager::SaveAudioState(bool sfx, bool vo, bool ambient, AudioSaveState& saveState)
{
    // Empty any existing stuff in the save state.
    saveState.playingSounds.clear();

    // If don't want to save anything, we can early out.
    if(!sfx && !vo && !ambient) { return; }

    // Iterate playing sounds to save each piece of audio (maybe).
    for(int i = mPlayingSounds.size() - 1; i >= 0; --i)
    {
        // If sound is ambient, but we don't want to include ambient in save state, ignore this sound!
        FMOD::ChannelGroup* channelGroup;
        mPlayingSounds[i].channel->getChannelGroup(&channelGroup);

        // Ignore audio channels that shouldn't be included in the save state.
        if(!sfx && channelGroup == mSFXChannelGroup)
        {
            continue;
        }
        if(!vo && channelGroup == mVOChannelGroup)
        {
            continue;
        }
        if(!ambient &&
           (channelGroup == mAmbientChannelGroup ||
            channelGroup == mMusicChannelGroup))
        {
            continue;
        }

        // Pause sound.
        mPlayingSounds[i].Pause();

        // Put it in the save state list.
        saveState.playingSounds.push_back(mPlayingSounds[i]);

        // Pop sound out of playing sounds list.
        std::swap(mPlayingSounds[i], mPlayingSounds.back());
        mPlayingSounds.pop_back();
    }
}

void AudioManager::RestoreAudioState(AudioSaveState& audioSaveState)
{
    // Resume playback of state channels.
    for(auto& sound : audioSaveState.playingSounds)
    {
        sound.Resume();
    }

    // Add back to playing sounds.
    // We'll say that restoring audio state *does not* clear other playing audio, so just append to existing playing channels.
    mPlayingSounds.insert(mPlayingSounds.end(), audioSaveState.playingSounds.begin(), audioSaveState.playingSounds.end());
}

FMOD::Sound* AudioManager::CreateSound(Audio* audio, AudioType audioType, bool is3D, bool isLooping)
{
    // If we've already got an FMOD sound instance for this Audio, use that.
    // NOTE: we're assuming previous audio data was loaded with same "is3D" and "isLooping" flags.
    // NOTE: if that's not the case in the future, may need to revise this.
    auto it = mFmodAudioData.find(audio);
    if(it != mFmodAudioData.end())
    {
        return it->second;
    }

    // Need to pass FMOD the length of audio data.
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = audio->GetDataBufferLength();

    // Determine flags.
    FMOD_MODE mode = FMOD_OPENMEMORY; // treat passed pointer as memory instead of a filename
    if(is3D)
    {
        mode |= FMOD_3D | FMOD_3D_LINEARSQUAREROLLOFF;
    }
    mode |= (isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

    // For music and ambient audio, stream it to avoid FPS drops when loading.
    // To stream the audio, we need to make sure the streaming buffer is never deleted while we're using it.
    // To achieve this, I'll just make a copy of the audio data.
    uint8_t* audioBuffer = audio->GetDataBuffer();
    if(audioType == AudioType::Ambient || audioType == AudioType::Music)
    {
        mode |= FMOD_CREATESTREAM;
        audioBuffer = new uint8_t[audio->GetDataBufferLength()];
        memcpy(audioBuffer, audio->GetDataBuffer(), exinfo.length);
    }

    // Create the sound using the audio data buffer.
    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = mSystem->createSound(reinterpret_cast<char*>(audioBuffer), mode, &exinfo, &sound);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD sound: %s", FMOD_ErrorString(result));
        return nullptr;
    }

    // If we made a copy of the audio data (for streaming audio), save it as userdata so we can delete it later.
    if(audioBuffer != audio->GetDataBuffer())
    {
        sound->setUserData(audioBuffer);
    }

    // Cache sound for reuse if this Audio is played again.
    mFmodAudioData[audio] = sound;

    // Return sound.
    return sound;
}

void AudioManager::DestroySound(FMOD::Sound* sound)
{
    // If userdata was set for this sound, it is audio data that was created for this sound.
    // Since the sound is being destroyed, the associated audio data can also be destroyed.
    void* userData = nullptr;
    sound->getUserData(&userData);

    // Only release the sound if the system is valid.
    // In the case of cleaning up after shutdown, the system has already been released, so the sound has also been released.
    if(mSystem != nullptr)
    {
        sound->release();
    }

    // Destroy the audio data buffer associated with the sound, if any.
    if(userData != nullptr)
    {
        uint8_t* audioData = static_cast<uint8_t*>(userData);
        delete[] audioData;
    }
}

FMOD::ChannelGroup* AudioManager::GetChannelGroupForAudioType(AudioType audioType) const
{
    switch(audioType)
    {
    default:
    case AudioType::SFX:
        return mSFXChannelGroup;
    case AudioType::VO:
        return mVOChannelGroup;
    case AudioType::Ambient:
        return mAmbientChannelGroup;
    case AudioType::Music:
        return mMusicChannelGroup;
    }
}

FMOD::Channel* AudioManager::CreateChannel(FMOD::Sound* sound, FMOD::ChannelGroup* channelGroup)
{
    // Calling "playSound" creates the channel in the appropriate channel group.
    // However, the name of the function is a bit misleading - we just create the channel, we don't play it yet (we pass true for paused arg).
    // This is important - if you want to set 3D attributes, you must set those attributes BEFORE unpausing the channel!
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(sound, channelGroup, true, &channel);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Failed to create FMOD channel: %s", FMOD_ErrorString(result));
    }
    return channel;
}

float AudioManager::GetVolumeMultiplierForAudioType(AudioType audioType) const
{
    // Get volume multiplier for audio type.
    switch(audioType)
    {
    case AudioType::SFX:
        return kSFXVolumeMultiplier;
    case AudioType::VO:
        return kVOVolumeMultiplier;
    case AudioType::Ambient:
        return kAmbientVolumeMultiplier;
    case AudioType::Music:
        return kMusicVolumeMultiplier;
    default:
        return 1.0f;
    }
}