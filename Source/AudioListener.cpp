//
// AudioListener.cpp
//
// Clark Kromenaker
//
#include "AudioListener.h"

#include "Vector3.h"

#include "Actor.h"
#include "Services.h"



TYPE_DEF_CHILD(Component, AudioListener);

AudioListener::AudioListener(Actor* owner) : Component(owner)
{
    
}

void AudioListener::Update(float deltaTime)
{
    Services::GetAudio()->UpdateListener(mOwner->GetPosition(), Vector3::Zero, mOwner->GetForward(), Vector3::UnitY);
}
