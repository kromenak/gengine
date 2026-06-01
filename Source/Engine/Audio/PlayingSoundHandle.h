//
// Clark Kromenaker
//
// A handle representing a playing sound. Returned by AudioManager.
//
// After sound stops/finishes, the handle is no longer valid. But it can still be passed around, used, and stored just fine.
// Calling functions on an invalid handle will have no effect.
//
#pragma once
#include <functional>

#include <fmod.hpp>

#include "Vector3.h"

class PlayingSoundHandle
{
public:
    PlayingSoundHandle() = default;
    PlayingSoundHandle(FMOD::Channel* channel, FMOD::Sound* sound);

    void Stop(float fadeOutTime = 0.0f);
    void Pause();
    void Resume();
    bool IsPlaying() const;

    void SetVolume(float volume);
    void SetPosition(const Vector3& position);

private:
    friend class AudioManager; // Allow audio manager to access internals.

    // An FMOD "channel" represents a single playing sound. Channel* is returned by PlaySound.
    // Once returned, this pointer is always valid, even if the sound stops playing or the channel is reused.
    // In one of those scenarios, calls to channel functions start to return FMOD_ERR_INVALID_HANDLE or similar result codes.
    FMOD::Channel* channel = nullptr;

    // Sound data being played.
    FMOD::Sound* sound = nullptr;

    // Callback to execute when sound finishes playing (either naturally or via stop).
    std::function<void()> mFinishCallback = nullptr;

    // The frame this sound started on.
    uint32_t mStartFrame = 0;
};