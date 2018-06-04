//
// AudioListener.cpp
//
// Clark Kromenaker
//
#include "AudioListener.h"
#include "Services.h"
#include "Vector3.h"

AudioListener::AudioListener(Actor* owner) : Component(owner)
{
    
}

void AudioListener::Update(float deltaTime)
{
    Services::GetAudio()->UpdateListener(mOwner->GetPosition(), Vector3::Zero, mOwner->GetForward(), Vector3::UnitY);
}
