//
// Clark Kromenaker
//
// Helper for setting timers that execute a callback when finished.
//
#include <cstdint>
#include <functional>
#include <vector>

class Timers
{
public:
    static void Update(float deltaTime);

    static void AddTimerSeconds(float seconds, const std::function<void()>& finishCallback);
    static void AddTimerMilliseconds(unsigned int milliseconds, const std::function<void()>& finishCallback);

    //TODO: Could be useful to have a way to cancel/remove a timer, perhaps using a handle system.

private:
    struct Timer
    {
        float secondsRemaining = 0.0f;
        std::function<void()> callback = nullptr;
    };
    static std::vector<Timer> mTimers;
};

// Helper for calculating delta time.
class DeltaTimer
{
public:
    float GetDeltaTime();

private:
    uint32_t mLastTicks = 0;
};