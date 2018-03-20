//
//  SDLAudio.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#include "SDLAudio.h"
#include <iostream>

bool SDLAudio::Initialize()
{
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
    
    /*
    music = Mix_LoadMUS("GRACER25.MP3");
    if(music == nullptr)
    {
        std::cout << "Mix_LoadMUS: " << Mix_GetError() << std::endl;
    }
    Mix_PlayMusic(music, 1);
    */
    
    // We initialized audio successfully!
    return true;
}

void SDLAudio::Shutdown()
{
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudio::Update(float deltaTime)
{
    
}

void SDLAudio::Play(Audio* audio)
{
    Play(audio, 0);
}

void SDLAudio::Play(Audio *audio, int fadeInMs)
{
    // Can't play nothin'!
    if(audio == nullptr) { return; }
    
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
}
