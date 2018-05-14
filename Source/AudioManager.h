//
//  AudioManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#pragma once
#include <SDL2/SDL.h>
#include <SDL2_mixer/SDL_mixer.h>

#include "fmod.hpp"

class Audio;
class Soundtrack;

class AudioManager
{
public:
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    
    void Play(Audio* audio);
    void Play(Audio* audio, int fadeInMs);
    
private:
    FMOD::System* mSystem = nullptr;
};
