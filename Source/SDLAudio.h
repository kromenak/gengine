//
//  SDLAudio.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//

#pragma once

#include "SDL/SDL.h"
#include "Audio.h"

class SDLAudio
{
public:
    
private:
    // The audio device ID obtained during initialization of the audio system.
    SDL_AudioDeviceID mDeviceId = 0;
    
    SDL_RWops* audioStream = nullptr;
unsigned int audioStreamLength = 0;
    unsigned int audioStreamPos = 0;
    
public:
    bool Initialize();
    void Shutdown();
    
    void Play(Audio& audio);
    
    void FillAudioBuffer(unsigned char* buffer, int bufferLength);
    
    SDL_RWops* GetAudioStream() { return audioStream; }
    unsigned int GetAudioStreamPos() { return audioStreamPos; }
    
private:
   
};
