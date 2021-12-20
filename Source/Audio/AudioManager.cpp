#include "AudioManager.h"

#include <iostream>

#include "Audio.h"
#include "GMath.h"
#include "Profiler.h"
#include "Vector3.h"

#include "Services.h"

PlayingSoundHandle::PlayingSoundHandle(FMOD::Channel* channel) :
    channel(channel)
{
    
}

void PlayingSoundHandle::Stop()
{
    if(channel != nullptr)
    {
        channel->stop();
        channel = nullptr;
    }
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

void Fader::Update(float deltaTime)
{
    if(fadeTimer < fadeDuration)
    {
        fadeTimer += deltaTime;

        // Set volume based on lerp.
        // Note that SetVolume can fail, but if so, we don't really care.
        float desiredAmbientVolume = Math::Lerp(fadeFrom, fadeTo, fadeTimer / fadeDuration);
        channelGroup->setVolume(desiredAmbientVolume);
    }
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
        channelGroup->getVolume(&fadeFrom);
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

        result = system->createChannelGroup(childName, &faders[i].channelGroup);
        if(result != FMOD_OK)
        {
            std::cout << FMOD_ErrorString(result) << std::endl;
            return false;
        }

        // Make the fade channel group an input to the ambient channel group.
        result = channelGroup->addGroup(faders[i].channelGroup);
        if(result != FMOD_OK)
        {
            std::cout << FMOD_ErrorString(result) << std::endl;
            return false;
        }
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
    // Fade current ambient to zero.
    faders[fadeIndex].SetFade(1.0f, 0.0f);

    // Go to next ambient fade group.
    fadeIndex++;
    fadeIndex %= 2;

    // Fade in new group.
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
    
    // Create channel groups for ambient crossfade.
    mAmbientCrossfade.Init(mSystem, "Ambient");

    // Create channel groups for music crossfade.
    mMusicCrossfade.Init(mSystem, "Music");

    // Get master channel group.
    result = mSystem->getMasterChannelGroup(&mMasterChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    // Set volumes for each audio type.
    // This ensures any volume multipliers have been applied.
    //TODO: Apply player preferences for audio levels instead of always 1.0f.
    SetVolume(AudioType::SFX, 1.0f);
    SetVolume(AudioType::VO, 1.0f);
    SetVolume(AudioType::Ambient, 1.0f);
    SetVolume(AudioType::Music, 1.0f);
    
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

    // Update crossfades.
    mAmbientCrossfade.Update(deltaTime);
    mMusicCrossfade.Update(deltaTime);

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
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }

    PlayingSoundHandle& soundHandle = CreateAndPlaySound(audio, AudioType::SFX);
    soundHandle.mFinishCallback = finishCallback;
    return soundHandle;
}

PlayingSoundHandle AudioManager::PlaySFX3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound3D(audio, AudioType::SFX, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayVO(Audio* audio)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound(audio, AudioType::VO);
}

PlayingSoundHandle AudioManager::PlayVO3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound3D(audio, AudioType::VO, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayAmbient(Audio* audio, float fadeInTime)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound(audio, AudioType::Ambient);
}

PlayingSoundHandle AudioManager::PlayAmbient3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound3D(audio, AudioType::Ambient, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayMusic(Audio* audio, float fadeInTime)
{
    if(audio == nullptr) { return PlayingSoundHandle(nullptr); }
    return CreateAndPlaySound(audio, AudioType::Music);
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
                sound.Stop();
                return;
            }
        }
    }
}

void AudioManager::StopAll()
{
    for(auto& sound : mPlayingSounds)
    {
        sound.Stop();
    }
    mPlayingSounds.clear();
}

void AudioManager::SwapAmbient()
{
    mAmbientCrossfade.Swap();
    mMusicCrossfade.Swap();
}

void AudioManager::SetMasterVolume(float volume)
{
    // Set volume. FMOD expects a normalized 0-1 value.
    mMasterChannelGroup->setVolume(Math::Clamp(volume, 0.0f, 1.0f));
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
        break;
    case AudioType::VO:
        multiplier = kVOVolumeMultiplier;
        break;
    case AudioType::Ambient:
        multiplier = kAmbientVolumeMultiplier;
        break;
    case AudioType::Music:
        multiplier = kMusicVolumeMultiplier;
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
           (channelGroup == mAmbientCrossfade.faders[0].channelGroup ||
            channelGroup == mAmbientCrossfade.faders[1].channelGroup ||
            channelGroup == mMusicCrossfade.faders[0].channelGroup ||
            channelGroup == mMusicCrossfade.faders[1].channelGroup))
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
        return forVolume ? mAmbientCrossfade.channelGroup : mAmbientCrossfade.GetActive();
    case AudioType::Music:
        return forVolume ? mMusicCrossfade.channelGroup : mMusicCrossfade.GetActive();
    }
}

PlayingSoundHandle& AudioManager::CreateAndPlaySound(Audio* audio, AudioType audioType, bool is3D)
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
        return mInvalidSoundHandle;
    }
    
    // Play the sound, which returns the channel being played on.
    FMOD::Channel* channel = nullptr;
    result = mSystem->playSound(sound, GetChannelGroupForAudioType(audioType, false), false, &channel);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return mInvalidSoundHandle;
    }
    
    // Add to playing channels.
    mPlayingSounds.emplace_back(channel);
    mPlayingSounds.back().audio = audio;
    
    // Return channel being played on.
    return mPlayingSounds.back();
}

PlayingSoundHandle& AudioManager::CreateAndPlaySound3D(Audio* audio, AudioType audioType, const Vector3 &position, float minDist, float maxDist)
{
    PlayingSoundHandle& soundHandle = CreateAndPlaySound(audio, audioType, true);
    
    // Assuming sound is assigned to a channel successfully, set 3D attributes.
    if(soundHandle.channel != nullptr)
    {
        // Sometimes, callers may pass negative values to mean "use default" for min/max dists.
        if(minDist < 0.0f) { minDist = kDefault3DMinDist; }
        if(maxDist < 0.0f) { maxDist = kDefault3DMaxDist; }

        // Make sure max dist isn't invalid.
        if(maxDist < minDist) { maxDist = minDist; }

        // Set min/max distance.
        soundHandle.channel->set3DMinMaxDistance(minDist, maxDist);

        // Put at desired position. No velocity right now.
        soundHandle.channel->set3DAttributes((const FMOD_VECTOR*)&position, (const FMOD_VECTOR*)&Vector3::Zero);
    }
    return soundHandle;
}