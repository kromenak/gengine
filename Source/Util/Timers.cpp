#include "Timers.h"

#include <SDL.h>

/*static*/ std::vector<Timers::Timer> Timers::mTimers;

/*static*/ void Timers::AddTimerSeconds(float seconds, const std::function<void()>& finishCallback)
{
    // See if we can reuse a previously created timer.
    //TODO: Obviously iterating like this could become problematic if we have a ton of timers...but it might be fine for now.
    for(auto& timer : mTimers)
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
    mTimers.push_back(timer);
}

/*static*/ void Timers::AddTimerMilliseconds(unsigned int milliseconds, const std::function<void()>& finishCallback)
{
    return AddTimerSeconds(static_cast<float>(milliseconds) * 0.001f, finishCallback);
}

/*static*/ void Timers::Update(float deltaTime)
{
    // TODO: What if a timer's callback adds a timer to this list?
    for(auto& timer : mTimers)
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

float DeltaTimer::GetDeltaTime()
{
    // Limit to ~60FPS.
    // If we get here again and 16ms have not passed, we wait.
    while(SDL_GetTicks() < mLastTicks + 16) { }

    // Calculate the time delta.
    uint32_t currentTicks = SDL_GetTicks();
    uint32_t deltaTicks = currentTicks - mLastTicks;
    float deltaTime = static_cast<float>(deltaTicks) * 0.001f;

    // Save last ticks for next frame.
    mLastTicks = currentTicks;

    // Limit the time delta. At least 0s, and at most, 0.05s.
    if(deltaTime < 0.0f) { deltaTime = 0.0f; }
    if(deltaTime > 0.05f) { deltaTime = 0.05f; }

    // That's it!
    return deltaTime;
}