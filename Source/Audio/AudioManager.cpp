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
    
    // Get master channel group.
    result = mSystem->getMasterChannelGroup(&mMasterChannelGroup);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return;
    }
    
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
    mSystem->update();
    
    // See if any playing channels are no longer playing.
    for(int i = mPlayingChannels.size() - 1; i >= 0; --i)
    {
        bool isPlaying = false;
        FMOD_RESULT result = mPlayingChannels[i]->isPlaying(&isPlaying);
        if((result == FMOD_OK && !isPlaying) || result == FMOD_ERR_INVALID_HANDLE)
        {
            // SWAP IT AND POP IT!
            std::swap(mPlayingChannels[i], mPlayingChannels.back());
            mPlayingChannels.pop_back();
        }
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

void AudioManager::PlaySFX(Audio* audio)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::SFX);
}

void AudioManager::PlaySFX3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::SFX, position, minDist, maxDist);
}

void AudioManager::PlayVO(Audio* audio)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::VO);
}

void AudioManager::PlayVO3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::VO, position, minDist, maxDist);
}

void AudioManager::PlayAmbient(Audio* audio, int fadeInMs)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::Ambient);
}

void AudioManager::PlayAmbient3D(Audio* audio, const Vector3 &position, float minDist, float maxDist)
{
    if(audio == nullptr) { return; }
    CreateAndPlaySound3D(audio->GetDataBuffer(), audio->GetDataBufferLength(), AudioType::Ambient, position, minDist, maxDist);
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
    
    // Set volume. FMOD expects a normalized 0-1 value.
    channelGroup->setVolume(Math::Clamp(volume, 0.0f, 1.0f));
}

float AudioManager::GetVolume(AudioType audioType) const
{
    FMOD::ChannelGroup* channelGroup = GetChannelGroupForAudioType(audioType);
    if(channelGroup == nullptr) { return 0.0f; }
    
    float volume = 0.0f;
    channelGroup->getVolume(&volume);
    return volume;
}

AudioState AudioManager::SaveAudioState()
{
    // Create state object.
    AudioState state;
    state.channels = mPlayingChannels;
    
    // Pause all playing sounds.
    for(auto& channel : mPlayingChannels)
    {
        channel->setPaused(true);
    }
    
    // Clear playing channels.
    // All saved channels are no longer playing until later restored.
    mPlayingChannels.clear();
    
    // Return audio state - up to the caller to store the state and restore it when it makes sense.
    return state;
}

void AudioManager::RestoreAudioState(const AudioState& audioState)
{
    // Resume playback of state channels.
    for(auto& channel : mPlayingChannels)
    {
        channel->setPaused(false);
    }
    
    // Add channels from state back to playing channels.
    // We'll say that restoring audio state *does not* clear other playing audio, so just append to existing playing channels.
    mPlayingChannels.insert(mPlayingChannels.end(), audioState.channels.begin(), audioState.channels.end());
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

FMOD::Channel* AudioManager::CreateAndPlaySound(const char* buffer, int bufferLength, AudioType audioType, bool is3D)
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
        return nullptr;
    }
    
    // Play the sound, which returns the channel being played on.
    FMOD::Channel* channel = nullptr;
    result = mSystem->playSound(sound, GetChannelGroupForAudioType(audioType), false, &channel);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return nullptr;
    }
    
    // Add to playing channels.
    mPlayingChannels.push_back(channel);
    
    // Return channel being played on.
    return channel;
}

FMOD::Channel* AudioManager::CreateAndPlaySound3D(const char* buffer, int bufferLength, AudioType audioType, const Vector3 &position, float minDist, float maxDist)
{
    FMOD::Channel* channel = CreateAndPlaySound(buffer, bufferLength, audioType, true);
    
    // Assuming sound is assigned to a channel successfully, set 3D attributes.
    if(channel != nullptr)
    {
        // Sometimes, callers may pass negative values to mean "use default" for min/max dists.
        if(minDist < 0.0f) { minDist = kDefault3DMinDist; }
        if(maxDist < 0.0f) { maxDist = kDefault3DMaxDist; }

        // Make sure max dist isn't invalid.
        if(maxDist < minDist) { maxDist = minDist; }
        
        // Set min/max distance.
        channel->set3DMinMaxDistance(minDist, maxDist);
        
        // Put at desired position. No velocity right now.
        channel->set3DAttributes((const FMOD_VECTOR*)&position, (const FMOD_VECTOR*)&Vector3::Zero);
    }
}
