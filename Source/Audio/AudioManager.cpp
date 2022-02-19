#include "AudioManager.h"

#include <iostream>

#include "Audio.h"
#include "GMath.h"
#include "Profiler.h"
#include "SaveManager.h"
#include "Services.h"
#include "Vector3.h"

PlayingSoundHandle::PlayingSoundHandle(FMOD::Channel* channel, Audio* audio) :
    channel(channel),
    audio(audio)
{
    
}

void PlayingSoundHandle::Stop(float fadeOutTime)
{
    Services::GetAudio()->Stop(*this, fadeOutTime);
}

void PlayingSoundHandle::Pause()
{
    if(channel != nullptr)
    {
        channel->setPaused(true);
    }
}

void PlayingSoundHandle::Resume()
{
    if(channel != nullptr)
    {
        channel->setPaused(false);
    }
}

void PlayingSoundHandle::SetVolume(float volume)
{
    // This may fail if the channel handle is no longer valid.
    // But that means we're trying to set volume for a sound that's not playing. So...it doesn't matter.
    if(channel != nullptr)
    {
        channel->setVolume(Math::Clamp(volume, 0.0f, 1.0f));
    }
}

bool PlayingSoundHandle::IsPlaying() const
{
    // No channel means not playing.
    if(channel == nullptr) { return false; }
    
    // Query whether channel is playing.
    bool isPlaying = false;
    FMOD_RESULT result = channel->isPlaying(&isPlaying);
    
    // If result is not OK, assume the sound is not playing.
    // Some reasons it might not be OK:
    //  - sound has stopped playing (FMOD_ERR_INVALID_HANDLE)
    //  - channel stolen b/c trying to play more sounds than there are channels (FMOD_ERR_CHANNEL_STOLEN)
    if(result != FMOD_OK)
    {
        return false;
    }
    
    // Assuming OK was returned, either the channel is playing or not!
    return isPlaying;
}

bool Fader::Update(float deltaTime)
{
    if(fadeTimer < fadeDuration)
    {
        fadeTimer += deltaTime;

        // Set volume based on lerp.
        // Note that SetVolume can fail, but if so, we don't really care.
        float volume = Math::Lerp(fadeFrom, fadeTo, fadeTimer / fadeDuration);
        channelControl->setVolume(volume);

        // After fading out completely, stop any sounds on this channel.
        // This stops long/looping sounds from continuing when fading back in.
        if(volume <= 0.0f)
        {
            channelControl->stop();
        }
    }
    return fadeTimer >= fadeDuration;
}

void Fader::SetFade(float fadeTime, float targetVolume, float startVolume)
{
    // Set duration and reset timer.
    fadeDuration = fadeTime;
    fadeTimer = 0.0f;

    // Save target volume.
    fadeTo = Math::Clamp(targetVolume, 0.0f, 1.0f);

    // If start volume is specified, use that value for "fade from".
    // If not specified, the current ambient volume is used.
    if(startVolume >= 0.0f)
    {
        fadeFrom = Math::Clamp(startVolume, 0.0f, 1.0f);
    }
    else
    {
        channelControl->getVolume(&fadeFrom);
    }
}

bool Crossfader::Init(FMOD::System* system, const char* name)
{
    // Create channel group. This inputs to master channel group by default.
    FMOD_RESULT result = system->createChannelGroup(name, &channelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    // Create fade channel groups that feed into the parent channel group.
    // So you end up with [Fade Channel Group] --> [Parent Channel Group] --> [Master Channel Group] --> [Your Ears].
    for(int i = 0; i < 2; ++i)
    {
        char childName[64];
        snprintf(childName, sizeof(childName), "%s %i", name, i);

        result = system->createChannelGroup(childName, &faderChannelGroups[i]);
        if(result != FMOD_OK)
        {
            std::cout << FMOD_ErrorString(result) << std::endl;
            return false;
        }

        // Make the fade channel group an input to the ambient channel group.
        result = channelGroup->addGroup(faderChannelGroups[i]);
        if(result != FMOD_OK)
        {
            std::cout << FMOD_ErrorString(result) << std::endl;
            return false;
        }

        // Tell associated fader to use this channel group when fading.
        faders[i].channelControl = faderChannelGroups[i];
    }

    // All's good.
    return true;
}

void Crossfader::Update(float deltaTime)
{
    // Update faders.
    for(int i = 0; i < 2; ++i)
    {
        faders[i].Update(deltaTime);
    }
}

void Crossfader::Swap()
{
    // Fade out current.
    faders[fadeIndex].SetFade(1.0f, 0.0f);

    // Go to next ambient fade group.
    fadeIndex++;
    fadeIndex %= 2;

    // Fade in next.
    faders[fadeIndex].SetFade(1.0f, 1.0f);
}

bool AudioManager::Initialize()
{
    TIMER_SCOPED("AudioManager::Initialize");

	// Create the FMOD system.
    FMOD_RESULT result = FMOD::System_Create(&mSystem);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
	
	// Retrieve the FMOD version.
    unsigned int version;
    result = mSystem->getVersion(&version);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
	
	// Verify that the FMOD library version matches the header version.
    if(version < FMOD_VERSION)
    {
        std::cout << "FMOD lib version " << version << " doesn't match header version " <<  FMOD_VERSION << std::endl;
        return false;
    }
	
	// Initialize the FMOD system.
    result = mSystem->init(32, FMOD_INIT_NORMAL, 0);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    // Create SFX channel group.
    result = mSystem->createChannelGroup("SFX", &mSFXChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    // Create VO channel group.
    result = mSystem->createChannelGroup("VO", &mVOChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    // Create ambient channel group.
    result = mSystem->createChannelGroup("Ambient", &mAmbientChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    // Create music channel group.
    result = mSystem->createChannelGroup("Music", &mMusicChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    // Get master channel group.
    result = mSystem->getMasterChannelGroup(&mMasterChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
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
    
    // We initialized audio successfully!
    return true;
}

void AudioManager::Shutdown()
{
	// Close and release FMOD system.
    FMOD_RESULT result = mSystem->close();
    result = mSystem->release();
}

void AudioManager::Update(float deltaTime)
{
    // Update FMOD system every frame.
    mSystem->update();

    // Update faders.
    for(int i = 0; i < mFaders.size(); ++i)
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
    
    /*
    // For testing fade in/out behavior.
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_M))
    {
        mAmbientFadeChannelGroups[mCurrentAmbientIndex].SetFade(1.0f, 1.0f);
    }
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_N))
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
        std::cout << FMOD_ErrorString(result) << std::endl;
    }
}

PlayingSoundHandle AudioManager::PlaySFX(Audio* audio, std::function<void()> finishCallback)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }

    PlayingSoundHandle& soundHandle = CreateAndPlaySound2D(audio, AudioType::SFX);
    soundHandle.mFinishCallback = finishCallback;
    return soundHandle;
}

PlayingSoundHandle AudioManager::PlaySFX3D(Audio* audio, const Vector3& position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }
    return CreateAndPlaySound3D(audio, AudioType::SFX, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayVO(Audio* audio)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }
    return CreateAndPlaySound2D(audio, AudioType::VO);
}

PlayingSoundHandle AudioManager::PlayVO3D(Audio* audio, const Vector3& position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }
    return CreateAndPlaySound3D(audio, AudioType::VO, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayAmbient(Audio* audio, float fadeInTime)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }
    return CreateAndPlaySound2D(audio, AudioType::Ambient);
}

PlayingSoundHandle AudioManager::PlayAmbient3D(Audio* audio, const Vector3& position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }
    return CreateAndPlaySound3D(audio, AudioType::Ambient, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayMusic(Audio* audio, float fadeInTime)
{
    if(audio == nullptr) { return PlayingSoundHandle(); }

    PlayingSoundHandle& soundHandle = CreateAndPlaySound2D(audio, AudioType::Music);
    if(!Math::IsZero(fadeInTime))
    {
        mFaders.emplace_back(soundHandle.channel);
        mFaders.back().SetFade(fadeInTime, 1.0f, 0.0f);
    }
    return soundHandle;
}

void AudioManager::Stop(Audio* audio)
{
    if(audio != nullptr)
    {
        for(auto& sound : mPlayingSounds)
        {
            if(sound.audio == audio)
            {
                // After stopping, sound is removed from playing sounds during next update loop.
                Stop(sound);
                return;
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

void AudioManager::StopOnOrAfterFrame(uint32 frame)
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
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType, true);
    if(channelGroup == nullptr) { return; }

    // Clamp input volume to 0-1 range.
    // Do this before applying multiplier to avoid user passing in like 200 and avoiding multiplier effects.
    volume = Math::Clamp(volume, 0.0f, 1.0f);

    // Get volume multiplier for audio type.
    float multiplier = 1.0f;
    switch(audioType)
    {
    case AudioType::SFX:
        multiplier = kSFXVolumeMultiplier;
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_SFX_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::VO:
        multiplier = kVOVolumeMultiplier;
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_VO_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::Ambient:
        multiplier = kAmbientVolumeMultiplier;
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_AMBIENT_VOLUME, static_cast<int>(volume * 100));
        break;
    case AudioType::Music:
        multiplier = kMusicVolumeMultiplier;
        gSaveManager.GetPrefs()->Set(PREFS_SOUND, PREFS_MUSIC_VOLUME, static_cast<int>(volume * 100));
    default:
        multiplier = 1.0f;
        break;
    }
    
    // Set volume. FMOD expects a normalized 0-1 value.
    channelGroup->setVolume(Math::Clamp(volume * multiplier, 0.0f, 1.0f));
}

float AudioManager::GetVolume(AudioType audioType) const
{
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType, true);
    if(channelGroup == nullptr) { return 0.0f; }
    
    float volume = 0.0f;
    channelGroup->getVolume(&volume);
    return volume;
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
    GetChannelGroupForAudioType(audioType, true)->setMute(mute);

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
    GetChannelGroupForAudioType(audioType, true)->getMute(&mute);
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

FMOD::ChannelGroup* AudioManager::GetChannelGroupForAudioType(AudioType audioType, bool forVolume) const
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

PlayingSoundHandle& AudioManager::CreateAndPlaySound2D(Audio* audio, AudioType audioType)
{
    // Create the sound from the audio buffer.
    FMOD::Sound* sound = CreateSound(audio, false);
    if(sound == nullptr)
    {
        return mInvalidSoundHandle;
    }

    // Create the channel that will play the sound in the correct channel group.
    FMOD::Channel* channel = CreateChannel(sound, GetChannelGroupForAudioType(audioType, false));
    if(channel == nullptr)
    {
        return mInvalidSoundHandle;
    }

    // For 2D audio, we don't need to set any additional attributes. So, just unpause it right away.
    channel->setPaused(false);

    // Create and return sound handle.
    mPlayingSounds.emplace_back(channel, audio);
    mPlayingSounds.back().mStartFrame = GEngine::Instance()->GetFrameNumber();
    return mPlayingSounds.back();
}

PlayingSoundHandle& AudioManager::CreateAndPlaySound3D(Audio* audio, AudioType audioType, const Vector3 &position, float minDist, float maxDist)
{
    // Create the 3D sound from the audio buffer.
    FMOD::Sound* sound = CreateSound(audio, true);
    if(sound == nullptr)
    {
        return mInvalidSoundHandle;
    }

    // Create the channel that will play the sound in the correct channel group.
    FMOD::Channel* channel = CreateChannel(sound, GetChannelGroupForAudioType(audioType, false));
    if(channel == nullptr)
    {
        return mInvalidSoundHandle;
    }

    // Sometimes, callers may pass negative values to mean "use default" for min/max dists.
    if(minDist < 0.0f) { minDist = kDefault3DMinDist; }
    if(maxDist < 0.0f) { maxDist = kDefault3DMaxDist; }

    // Make sure min/max dist are in valid ranges.
    if(maxDist < minDist) { maxDist = minDist; }
    if(minDist > maxDist) { minDist = maxDist; }

    // Set distance attributes.
    channel->set3DMinMaxDistance(minDist, maxDist);

    // Set position.
    channel->set3DAttributes((const FMOD_VECTOR*)&position, nullptr);

    // Play the sound. Important to do this AFTER setting 3D attributes for correct results.
    channel->setPaused(false);

    // Create and return sound handle.
    mPlayingSounds.emplace_back(channel, audio);
    mPlayingSounds.back().mStartFrame = GEngine::Instance()->GetFrameNumber();
    return mPlayingSounds.back();
}

FMOD::Sound* AudioManager::CreateSound(Audio* audio, bool is3D)
{
    // Need to pass FMOD the length of audio data.
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = audio->GetDataBufferLength();

    // Determine flags.
    FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_LOOP_OFF;
    if(is3D)
    {
        mode |= FMOD_3D | FMOD_3D_LINEARSQUAREROLLOFF;
    }

    // Create the sound using the audio buffer.
    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = mSystem->createSound(audio->GetDataBuffer(), mode, &exinfo, &sound);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
    }
    return sound;
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
        std::cout << FMOD_ErrorString(result) << std::endl;
    }
    return channel;
}