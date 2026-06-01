#include "Fader.h"

#include <fmod_errors.h>

#include "GMath.h"
#include "ReportManager.h"

bool Fader::Update(float deltaTime)
{
    if(fadeTimer < fadeDuration)
    {
        fadeTimer += deltaTime;

        // Set volume based on lerp.
        // Note that SetVolume can fail, but if so, we don't really care.
        float volume = Math::Lerp(fadeFrom, fadeTo, fadeTimer / fadeDuration);
        channelControl->setVolume(volume);

        // After fading out completely, stop any sounds on this channel.
        // This stops long/looping sounds from continuing when fading back in.
        if(volume <= 0.0f)
        {
            channelControl->stop();
        }
    }
    return fadeTimer >= fadeDuration;
}

void Fader::SetFade(float fadeTime, float targetVolume, float startVolume)
{
    // Set duration and reset timer.
    fadeDuration = fadeTime;
    fadeTimer = 0.0f;

    // Save target volume.
    fadeTo = Math::Clamp(targetVolume, 0.0f, 1.0f);

    // If start volume is specified, use that value for "fade from".
    // If not specified, the current ambient volume is used.
    if(startVolume >= 0.0f)
    {
        fadeFrom = Math::Clamp(startVolume, 0.0f, 1.0f);
    }
    else
    {
        channelControl->getVolume(&fadeFrom);
    }
}

bool Crossfader::Init(FMOD::System* system, const char* name)
{
    // Create channel group. This inputs to master channel group by default.
    FMOD_RESULT result = system->createChannelGroup(name, &channelGroup);
    if(result != FMOD_OK)
    {
        LOG_ERROR("Can't create FMOD channel group %s: %s", name, FMOD_ErrorString(result));
        return false;
    }

    // Create fade channel groups that feed into the parent channel group.
    // So you end up with [Fade Channel Group] --> [Parent Channel Group] --> [Master Channel Group] --> [Your Ears].
    for(int i = 0; i < 2; ++i)
    {
        char childName[64];
        snprintf(childName, sizeof(childName), "%s %i", name, i);

        result = system->createChannelGroup(childName, &faderChannelGroups[i]);
        if(result != FMOD_OK)
        {
            LOG_ERROR("Can't create FMOD channel group %s: %s", childName, FMOD_ErrorString(result));
            return false;
        }

        // Make the fade channel group an input to the channel group.
        result = channelGroup->addGroup(faderChannelGroups[i]);
        if(result != FMOD_OK)
        {
            LOG_ERROR("Can't add FMOD channel group: %s", FMOD_ErrorString(result));
            return false;
        }

        // Tell associated fader to use this channel group when fading.
        faders[i].channelControl = faderChannelGroups[i];
    }

    // All's good.
    return true;
}

void Crossfader::Update(float deltaTime)
{
    // Update faders.
    for(auto& fader : faders)
    {
        fader.Update(deltaTime);
    }
}

void Crossfader::Swap()
{
    // Fade out current.
    faders[fadeIndex].SetFade(1.0f, 0.0f);

    // Go to next ambient fade group.
    fadeIndex++;
    fadeIndex %= 2;

    // Fade in next.
    faders[fadeIndex].SetFade(1.0f, 1.0f);
}