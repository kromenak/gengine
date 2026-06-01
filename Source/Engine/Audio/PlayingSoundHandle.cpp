#include "PlayingSoundHandle.h"

#include <fmod_errors.h>

#include "AudioManager.h"
#include "GEngine.h"
#include "ReportManager.h"

PlayingSoundHandle::PlayingSoundHandle(FMOD::Channel* channel, FMOD::Sound* sound) :
    channel(channel),
    sound(sound)
{
    mStartFrame = GEngine::Instance()->GetFrameNumber();
}

void PlayingSoundHandle::Stop(float fadeOutTime)
{
    gAudioManager.Stop(*this, fadeOutTime);
}

void PlayingSoundHandle::Pause()
{
    if(channel != nullptr)
    {
        FMOD_RESULT result = channel->setPaused(true);
        if(result != FMOD_OK)
        {
            LOG_ERROR("FMOD failed to pause channel: %s", FMOD_ErrorString(result));
        }
    }
}

void PlayingSoundHandle::Resume()
{
    if(channel != nullptr)
    {
        FMOD_RESULT result = channel->setPaused(false);
        if(result != FMOD_OK)
        {
            LOG_ERROR("FMOD failed to resume channel: %s", FMOD_ErrorString(result));
        }
    }
}

bool PlayingSoundHandle::IsPlaying() const
{
    // No channel means not playing.
    if(channel == nullptr) { return false; }

    // Query whether channel is playing.
    bool isPlaying = false;
    FMOD_RESULT result = channel->isPlaying(&isPlaying);

    // If result is not OK, assume the sound is not playing.
    // Some reasons it might not be OK:
    //  - sound has stopped playing (FMOD_ERR_INVALID_HANDLE)
    //  - channel stolen b/c trying to play more sounds than there are channels (FMOD_ERR_CHANNEL_STOLEN)
    if(result != FMOD_OK)
    {
        return false;
    }

    // Assuming OK was returned, either the channel is playing or not!
    return isPlaying;
}

void PlayingSoundHandle::SetVolume(float volume)
{
    // This may fail if the channel handle is no longer valid.
    // But that means we're trying to set volume for a sound that's not playing. So...it doesn't matter.
    if(channel != nullptr)
    {
        channel->setVolume(Math::Clamp(volume, 0.0f, 1.0f));
    }
}

void PlayingSoundHandle::SetPosition(const Vector3& position)
{
    if(channel != nullptr)
    {
        channel->set3DAttributes((const FMOD_VECTOR*)&position, nullptr);
    }
}