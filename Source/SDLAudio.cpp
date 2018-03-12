//
//  SDLAudio.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#include "SDLAudio.h"
#include <iostream>

void SDLAudioCallback(void* userdata, unsigned char* stream, int len)
{
    // If user data isn't set, we can't play anything.
    if(userdata == nullptr)
    {
        SDL_memset(stream, 200, len);
        return;
    }
    
    // Get pointer to SDLAudio.
    SDLAudio* audio = (SDLAudio*)userdata;
    audio->FillAudioBuffer(stream, len);
}

bool SDLAudio::Initialize()
{
    // Init audio subsystem.
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        return false;
    }
    
    int flags = MIX_INIT_MP3;
    int result = Mix_Init(flags);
    if((result & flags) != flags)
    {
        std::cout << "Mix_Init: failed to init MP3 support." << std::endl;
        std::cout << "Mix_Init: " << Mix_GetError() << std::endl;
    }
    
    /*
    // Fill in want structure with our desired audio device config.
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 4096;
    want.callback = SDLAudioCallback;
    want.userdata = this;
    
    // Attempt to open device. TODO: Currently using default options.
    mDeviceId = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(mDeviceId == 0)
    {
        std::cout << "Couldn't open audio device." << std::endl;
        return false;
    }
    
    //TODO: Verify that obtained audio device meets our needs.
    
    // Tell audio system it can start playing audio.
    SDL_PauseAudioDevice(mDeviceId, 0);
    */
     
    // We initialized audio successfully!
    return true;
}

void SDLAudio::Shutdown()
{
    SDL_PauseAudioDevice(mDeviceId, 1);
    SDL_CloseAudioDevice(mDeviceId);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudio::Play(Audio &audio)
{
    if(audioStream != nullptr)
    {
        SDL_FreeRW(audioStream);
        audioStream = nullptr;
    }
    audioStream = SDL_RWFromMem(audio.GetDataBuffer(), audio.GetDataBufferLength());
    audioStreamLength = audio.GetDataBufferLength();
    audioStreamPos = 0;
}

void SDLAudio::FillAudioBuffer(unsigned char *buffer, int bufferLength)
{
    // If no audio to play, just fill with silence.
    if(audioStream == nullptr)
    {
        SDL_memset(buffer, 0, bufferLength);
        return;
    }
    
    // If there's more audio available than the size of the buffer, just fill the whole buffer.
    if(audioStreamPos + bufferLength < audioStreamLength)
    {
        SDL_memcpy(buffer, audioStream, bufferLength);
        audioStreamPos += bufferLength;
    }
    else
    {
        unsigned int fillLength = audioStreamLength - audioStreamPos;
        
        SDL_memcpy(buffer, audioStream, fillLength);
        unsigned char* silenceBuffer = buffer + fillLength;
        SDL_memset(silenceBuffer, 0, bufferLength - fillLength);
        
        // Since we've played the entire audio stream, we can now delete it.
        SDL_FreeRW(audioStream);
        audioStream = nullptr;
    }
}



