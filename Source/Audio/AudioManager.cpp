//
//  AudioManager.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#include "AudioManager.h"

#include <iostream>

#include "Audio.h"
#include "GMath.h"
#include "Vector3.h"

PlayingSoundHandle::PlayingSoundHandle(FMOD::Channel* channel) :
    channel(channel)
{
    
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

bool AudioManager::Initialize()
{
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
    
    // Create ambient fade in/out channel group.
    result = mSystem->createChannelGroup("Ambient Fade", &mAmbientFadeChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    // Ambient fade in/out group must be a child of the ambient group.
    result = mAmbientFadeChannelGroup->addGroup(mAmbientChannelGroup);
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
        return;
    }
    
    // Set volumes for each audio type.
    // This ensures any volume multipliers have been applied.
    //TODO: Apply player preferences for audio levels instead of always 1.0f.
    SetVolume(AudioType::SFX, 1.0f);
    SetVolume(AudioType::VO, 1.0f);
    SetVolume(AudioType::Ambient, 1.0f);
    
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
    
    // See if any playing channels are no longer playing.
    for(int i = mPlayingSounds.size() - 1; i >= 0; --i)
    {
        if(!mPlayingSounds[i].IsPlaying())
        {
            std::swap(mPlayingSounds[i], mPlayingSounds.back());
            mPlayingSounds.pop_back();
        }
    }
    
    /*
    // For testing fade in/out behavior.
    if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_M))
    {
        AmbientFade(1.0f, 1.0f);
    }
    if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_N))
    {
        AmbientFade(1.0f, 0.0f);
    }
    */
    
    // Update ambient fade in and fade out.
    if(mAmbientFadeTimer < mAmbientFadeDuration)
    {
        mAmbientFadeTimer += deltaTime;
        
        // Set volume based on lerp.
        // Note that SetVolume can fail, but if so, we don't really care.
        float desiredAmbientVolume = Math::Lerp(mAmbientFadeFrom, mAmbientFadeTo, mAmbientFadeTimer / mAmbientFadeDuration);
        mAmbientFadeChannelGroup->setVolume(desiredAmbientVolume);
    }
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

PlayingSoundHandle AudioManager::PlaySFX(Audio* audio)
{
    if(audio == nullptr) { return; }
    return CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::SFX);
}

PlayingSoundHandle AudioManager::PlaySFX3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    return CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::SFX, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayVO(Audio* audio)
{
    if(audio == nullptr) { return; }
    return CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::VO);
}

PlayingSoundHandle AudioManager::PlayVO3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    return CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::VO, position, minDist, maxDist);
}

PlayingSoundHandle AudioManager::PlayAmbient(Audio* audio, float fadeInTime)
{
    if(audio == nullptr) { return; }
    
    // If specified, start fade in.
    if(fadeInTime > 0.0f)
    {
        AmbientFade(fadeInTime, 1.0f, 0.0f);
    }
    
    return CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::Ambient);
}

PlayingSoundHandle AudioManager::PlayAmbient3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    return CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::Ambient, position, minDist, maxDist);
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
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType);
    if(channelGroup == nullptr) { return; }
    
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
    default:
        multiplier = 1.0f;
        break;
    }
    
    // Set volume. FMOD expects a normalized 0-1 value.
    channelGroup->setVolume(Math::Clamp(volume * multiplier, 0.0f, 1.0f));
}

float AudioManager::GetVolume(AudioType audioType) const
{
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType);
    if(channelGroup == nullptr) { return 0.0f; }
    
    float volume = 0.0f;
    channelGroup->getVolume(&volume);
    return volume;
}

AudioSaveState AudioManager::SaveAudioState(bool includeAmbient)
{
    // Create state object.
    AudioSaveState state;
    
    // Iterate playing sounds to seach each piece of audio (maybe).
    for(int i = mPlayingSounds.size() - 1; i >= 0; --i)
    {
        // If sound is ambient, but we don't want to include ambient in save state, ignore this sound!
        FMOD::ChannelGroup* channelGroup;
        mPlayingSounds[i].channel->getChannelGroup(&channelGroup);
        if(channelGroup == mAmbientChannelGroup && !includeAmbient)
        {
            continue;
        }
        
        // Pause sound.
        mPlayingSounds[i].Pause();
        
        // Put it in the save state list.
        state.playingSounds.push_back(mPlayingSounds[i]);
        
        // Pop sound out of playing sounds list.
        std::swap(mPlayingSounds[i], mPlayingSounds.back());
        mPlayingSounds.pop_back();
    }
    
    // Return audio state - up to the caller to store the state and restore it when it makes sense.
    return state;
}

void AudioManager::RestoreAudioState(AudioSaveState& audioSaveState)
{
    // Resume playback of state channels.
    for(auto& sound : audioSaveState.playingSounds)
    {
        sound.Resume();
    }
    
    // Add channels from state back to playing channels.
    // We'll say that restoring audio state *does not* clear other playing audio, so just append to existing playing channels.
    mPlayingSounds.insert(mPlayingSounds.end(), audioSaveState.playingSounds.begin(), audioSaveState.playingSounds.end());
}

FMOD::ChannelGroup* AudioManager::GetChannelGroupForAudioType(AudioType audioType) const
{
    // Determine channel group for audio.
    FMOD::ChannelGroup* channelGroup = nullptr;
    switch(audioType)
    {
    default:
    case AudioType::SFX:
        channelGroup = mSFXChannelGroup;
        break;
    case AudioType::VO:
        channelGroup = mVOChannelGroup;
        break;
    case AudioType::Ambient:
        channelGroup = mAmbientChannelGroup;
        break;
    }
    return channelGroup;
}

PlayingSoundHandle AudioManager::CreateAndPlaySound(const char* buffer, int bufferLength, AudioType audioType, bool is3D)
{
    // Need to pass FMOD the length of audio data.
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = bufferLength;
    
    // Determine flags.
    FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_LOOP_OFF;
    if(is3D)
    {
        mode |= FMOD_3D | FMOD_3D_LINEARSQUAREROLLOFF;
    }
    
    // Create the sound using the audio buffer.
    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = mSystem->createSound(buffer, mode, &exinfo, &sound);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return PlayingSoundHandle(nullptr);
    }
    
    // Play the sound, which returns the channel being played on.
    FMOD::Channel* channel = nullptr;
    result = mSystem->playSound(sound, GetChannelGroupForAudioType(audioType), false, &channel);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return PlayingSoundHandle(nullptr);
    }
    
    // Add to playing channels.
    mPlayingSounds.emplace_back(channel);
    
    // Return channel being played on.
    return channel;
}

PlayingSoundHandle AudioManager::CreateAndPlaySound3D(const char* buffer, int bufferLength, AudioType audioType, const Vector3 &position, float minDist, float maxDist)
{
    PlayingSoundHandle soundInstance = CreateAndPlaySound(buffer, bufferLength, audioType, true);
    
    // Assuming sound is assigned to a channel successfully, set 3D attributes.
    if(soundInstance.channel != nullptr)
    {
        // Sometimes, callers may pass negative values to mean "use default" for min/max dists.
        if(minDist < 0.0f) { minDist = kDefault3DMinDist; }
        if(maxDist < 0.0f) { maxDist = kDefault3DMaxDist; }

        // Make sure max dist isn't invalid.
        if(maxDist < minDist) { maxDist = minDist; }
        
        // Set min/max distance.
        soundInstance.channel->set3DMinMaxDistance(minDist, maxDist);
        
        // Put at desired position. No velocity right now.
        soundInstance.channel->set3DAttributes((const FMOD_VECTOR*)&position, (const FMOD_VECTOR*)&Vector3::Zero);
    }
}

void AudioManager::AmbientFade(float fadeTime, float targetVolume, float startVolume)
{
    // Set duration and reset timer.
    mAmbientFadeDuration = fadeTime;
    mAmbientFadeTimer = 0.0f;
    
    // Save target volume.
    mAmbientFadeTo = Math::Clamp(targetVolume, 0.0f, 1.0f);
    
    // If start volume is specified, use that value for "fade from".
    // If not specified, the current ambient volume is used.
    if(startVolume >= 0.0f)
    {
        mAmbientFadeFrom = Math::Clamp(startVolume, 0.0f, 1.0f);
    }
    else
    {
        mAmbientFadeChannelGroup->getVolume(&mAmbientFadeFrom);
    }
}
