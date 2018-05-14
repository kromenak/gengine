//
//  AudioManager.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#include <iostream>
#include "AudioManager.h"
#include "Audio.h"
#include "fmod_errors.h"

bool AudioManager::Initialize()
{
    FMOD_RESULT result;
    result = FMOD::System_Create(&mSystem);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    uint version;
    result = mSystem->getVersion(&version);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    if(version < FMOD_VERSION)
    {
        std::cout << "FMOD lib version " << version << " doesn't match header version " <<  FMOD_VERSION << std::endl;
        return false;
    }
    
    void* extradriverdata = 0;
    result = mSystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
    if(result != FMOD_OK)
    {
        std::cout << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    
    /*
    // Init audio subsystem.
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        return false;
    }
    
    // Initialize mixer with MP3 support.
    int flags = MIX_INIT_MP3;
    int result = Mix_Init(flags);
    if((result & flags) != flags)
    {
        std::cout << "Mix_Init: " << Mix_GetError() << std::endl;
    }
    
    // Open audio with just default values.
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    */
    
    // We initialized audio successfully!
    return true;
}

void AudioManager::Shutdown()
{
    FMOD_RESULT result;
    result = mSystem->close();
    result = mSystem->release();
    
    //Mix_CloseAudio();
    //Mix_Quit();
    //SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioManager::Update(float deltaTime)
{
    mSystem->update();
}

void AudioManager::Play(Audio* audio)
{
    Play(audio, 0);
}

void AudioManager::Play(Audio* audio, int fadeInMs)
{
    // Can't play nothin'!
    if(audio == nullptr) { return; }
    
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = audio->GetDataBufferLength();
    
    FMOD::Channel* channel = nullptr;
    FMOD::Sound* sound1 = nullptr;
    FMOD_RESULT result;
    result = mSystem->createSound((const char*)audio->GetDataBuffer(), FMOD_OPENMEMORY | FMOD_LOOP_OFF, &exinfo, &sound1);
    
    result = mSystem->playSound(sound1, 0, false, &channel);
    /*
    // Decide whether this is music or a sound effect...our bulletproof method is whether it's an MP3 or not.
    SDL_RWops* audioData = SDL_RWFromMem(audio->GetDataBuffer(), audio->GetDataBufferLength());
    if(audio->IsMusic())
    {
        Mix_Music* music = Mix_LoadMUSType_RW(audioData, MUS_MP3, SDL_FALSE);
        if(fadeInMs > 0)
        {
            Mix_FadeInMusic(music, 1, fadeInMs);
        }
        else
        {
            Mix_PlayMusic(music, 1);
        }
    }
    else
    {
        Mix_Chunk* chunk = Mix_LoadWAV_RW(audioData, SDL_FALSE);
        if(fadeInMs > 0)
        {
            Mix_FadeInChannel(-1, chunk, 0, fadeInMs);
        }
        else
        {
            Mix_PlayChannel(-1, chunk, 0);
        }
    }
    */
}
