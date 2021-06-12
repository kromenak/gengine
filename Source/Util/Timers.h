//
// Timers.h
//
// Clark Kromenaker
//
// Helper for setting timers that execute a callback when finished.
//
#include <functional>
#include <vector>

class Timers
{
public:
    static void Update(float deltaTime);

    static void AddTimerSeconds(float seconds, std::function<void()> finishCallback);
    static void AddTimerMilliseconds(unsigned int milliseconds, std::function<void()> finishCallback);

    //TODO: Could be useful to have a way to cancel/remove a timer, perhaps using a handle system.

private:
    struct Timer
    {
        float secondsRemaining = 0.0f;
        std::function<void()> callback = nullptr;
    };
    static std::vector<Timer> mTimers;
};