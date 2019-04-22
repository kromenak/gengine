//
//  AudioManager.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/23/17.
//
#pragma once
#include <SDL2/SDL.h>
#include "fmod.hpp"

class Audio;
class Soundtrack;

class Vector3;
class Quaternion;

class AudioManager
{
public:
    bool Initialize();
    void Shutdown();
    
    void Update(float deltaTime);
    
    void Play(Audio* audio);
    void Play(Audio* audio, int fadeInMs);
    
	void Play3D(Audio* audio, const Vector3& position, float minDist, float maxDist);
    
    void UpdateListener(const Vector3& position, const Vector3& velocity, const Vector3& forward, const Vector3& up);
    
private:
    FMOD::System* mSystem = nullptr;
};
