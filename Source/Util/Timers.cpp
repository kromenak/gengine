#include "Timers.h"

/*static*/ std::vector<Timers::Timer> Timers::mTimers;

/*static*/ void Timers::AddTimerSeconds(float seconds, std::function<void()> finishCallback)
{
    // See if we can reuse a previously created timer.
    //TODO: Obviously iterating like this could become problematic if we have a ton of timers...but it might be fine for now.
    for(unsigned int i = 0; i < mTimers.size(); i++)
    {
        // Any timer with no callback is effectively useless...
        if(mTimers[i].callback == nullptr)
        {
            mTimers[i].secondsRemaining = seconds;
            mTimers[i].callback = finishCallback;
            return;
        }
    }

    // Add a new timer to the list.
    Timer timer;
    timer.secondsRemaining = seconds;
    timer.callback = finishCallback;
    mTimers.push_back(timer);
    return;
}

/*static*/ void Timers::AddTimerMilliseconds(unsigned int milliseconds, std::function<void()> finishCallback)
{
    return AddTimerSeconds(milliseconds / 1000.0f, finishCallback);
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