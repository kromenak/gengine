#include "Timers.h"

#include <vector>

#include <SDL.h>

#include "GMath.h"

namespace
{
    struct Timer
    {
        float secondsRemaining = 0.0f;
        std::function<void()> callback = nullptr;
    };
    std::vector<Timer> timers;
}

void Timers::Update(float deltaTime)
{
    // TODO: What if a timer's callback adds a timer to this list?
    for(Timer& timer : timers)
    {
        // Decrement timers with time remaining.
        if(timer.secondsRemaining > 0.0f)
        {
            timer.secondsRemaining -= deltaTime;
            //printf("Time remaining: %f\n", timer.secondsRemaining);

            // Timer is complete! Execute callbacks.
            if(timer.secondsRemaining <= 0.0f)
            {
                if(timer.callback != nullptr)
                {
                    auto callback = timer.callback;
                    timer.callback = nullptr;
                    callback();
                }
            }
        }
    }
}

void Timers::AddTimerSeconds(float seconds, const std::function<void()>& finishCallback)
{
    // If seconds is zero or less, assume the callback should just be called immediately
    // (Yes, the game does set a zero second timer on at least one occasion.)
    if(seconds <= 0.0f)
    {
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // See if we can reuse a previously created timer.
    for(Timer& timer : timers)
    {
        // Any timer with no callback is effectively useless...
        if(timer.callback == nullptr)
        {
            timer.secondsRemaining = seconds;
            timer.callback = finishCallback;
            return;
        }
    }

    // Add a new timer to the list.
    Timer timer;
    timer.secondsRemaining = seconds;
    timer.callback = finishCallback;
    timers.push_back(timer);
}

void Timers::AddTimerMilliseconds(uint32_t milliseconds, const std::function<void()>& finishCallback)
{
    return AddTimerSeconds(static_cast<float>(milliseconds) * 0.001f, finishCallback);
}

Stopwatch::Stopwatch()
{
    // Cache high resolution counter frequency, which doesn't change at runtime.
    mCounterFrequency = static_cast<float>(SDL_GetPerformanceFrequency());

    // Reset stopwatch to start tracking time from...RIGHT NOW!
    Reset();
}

void Stopwatch::Reset()
{
    // Record starting high resolution counter value.
    mStartCounter = SDL_GetPerformanceCounter();
}

float Stopwatch::GetMilliseconds() const
{
    // Convert to milliseconds.
    return GetSeconds() * 1000.0f;
}

float Stopwatch::GetSeconds() const
{
    // Get counter delta since stopwatch started.
    uint64_t counter = SDL_GetPerformanceCounter();
    uint64_t counterDelta = counter - mStartCounter;

    // Convert to seconds.
    return static_cast<float>(counterDelta) / mCounterFrequency;
}

DeltaTimer::DeltaTimer()
{
    // Cache high resolution counter frequency, which doesn't change at runtime.
    mCounterFrequency = static_cast<float>(SDL_GetPerformanceFrequency());

    // Record starting high resolution counter value.
    mLastCounter = SDL_GetPerformanceCounter();
}

float DeltaTimer::GetDeltaTime()
{
    // See how many counter values have passed since this function was last called.
    uint64_t counter = SDL_GetPerformanceCounter();
    uint64_t counterDelta = counter - mLastCounter;

    // Record counter value for use in next call.
    mLastCounter = counter;

    // Convert counter to seconds.
    return static_cast<float>(counterDelta) / mCounterFrequency;
}

float DeltaTimer::GetDeltaTimeWithFpsThrottle(uint32_t maxFps, float maxDeltaTime)
{
    // An FPS throttle means we don't want to proceed until enough time has passed.
    // Instead of running at un-throttled FPS (as fast as possible), we purposely slow down execution to achieve a consistent FPS.

    // We are tracking time based on the high resolution counter. See what counter value we need before we can move on.
    // If (seconds = count / freq), then (count = seconds * freq).
    float desiredTimeDelta = 1.0f / static_cast<float>(maxFps);
    uint64_t desiredCounterDelta = static_cast<uint64_t>(mCounterFrequency * desiredTimeDelta);

    // Wait until the counter delta is at least our desired counter delta.
    // Overshooting is possible, but not worth accounting for or dealing with.
    uint64_t counter = 0;
    uint64_t counterDelta = 0;
    do
    {
        counter = SDL_GetPerformanceCounter();
        counterDelta = counter - mLastCounter;
    }
    while(counterDelta < desiredCounterDelta);

    // Save current counter value for use next time we call this function.
    mLastCounter = counter;

    // Seconds passed is counter delta divided by frequency.
    // Clamp to max is mainly useful when debugging, so you don't get a super large delta time after pausing execution.
    return Math::Clamp(static_cast<float>(counterDelta) / mCounterFrequency, 0.0f, maxDeltaTime);
}