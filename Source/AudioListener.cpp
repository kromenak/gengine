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

void AudioListener::OnUpdate(float deltaTime)
{
	Transform* transform = GetOwner()->GetTransform();
    Services::GetAudio()->UpdateListener(transform->GetPosition(), Vector3::Zero, transform->GetForward(), transform->GetUp());
}
