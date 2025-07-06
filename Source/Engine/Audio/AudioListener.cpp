#include "AudioListener.h"

#include "Vector3.h"

#include "Actor.h"
#include "AudioManager.h"

TYPEINFO_INIT(AudioListener, Component, 4)
{

}

AudioListener::AudioListener(Actor* owner) : Component(owner)
{

}

void AudioListener::OnUpdate(float deltaTime)
{
    Transform* transform = GetOwner()->GetTransform();
    gAudioManager.UpdateListener(transform->GetPosition(), Vector3::Zero, transform->GetForward(), transform->GetUp());
}
