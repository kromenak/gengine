//
//  SDLAudio.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#pragma once
#include <SDL2/SDL.h>
#include <SDL2_mixer/SDL_mixer.h>
#include "Audio.h"

class Soundtrack;

class SDLAudio
{
public:
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    
    void Play(Audio* audio);
    void Play(Audio* audio, int fadeInMs);
    
private:
};
