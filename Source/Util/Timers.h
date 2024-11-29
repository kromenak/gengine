//
// Clark Kromenaker
//
// Helper for setting timers that execute a callback when finished.
// Also helpers for tracking the passage of time.
//
#pragma once
#include <cstdint>
#include <functional>

namespace Timers
{
    void Update(float deltaTime);

    void AddTimerSeconds(float seconds, const std::function<void()>& finishCallback);
    void AddTimerMilliseconds(uint32_t milliseconds, const std::function<void()>& finishCallback);

    //TODO: Could be useful to have a way to cancel/remove a timer, perhaps using a handle system.
};

// A Stopwatch allows you to track how much time has passed since it was created or reset.
class Stopwatch
{
public:
    Stopwatch();
    void Reset();
    float GetMilliseconds() const;
    float GetSeconds() const;

private:
    // The frequency of the high resolution counter; cached to avoid retrieving it every call.
    // Per docs, this value doesn't change during program execution.
    float mCounterFrequency = 0.0f;

    // The high resolution counter value when the stopwatch was created/reset.
    uint64_t mStartCounter = 0L;
};

// The DeltaTimer can be used to calculate how much time has passed since time was last queried.
class DeltaTimer
{
public:
    DeltaTimer();
    float GetDeltaTime();
    float GetDeltaTimeWithFpsThrottle(uint32_t maxFps, float maxDeltaTime);

private:
    // The frequency of the high resolution counter; cached to avoid retrieving it every call.
    // Per docs, this value doesn't change during program execution.
    float mCounterFrequency = 0.0f;

    // The high resolution counter value when delta time was last calculated.
    uint64_t mLastCounter = 0L;
};