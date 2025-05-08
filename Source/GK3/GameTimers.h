//
// Clark Kromenaker
//
// Game timers enable a noun/verb action to execute after some time has passed.
// These timers can persist between scenes, so they need to be somewhere centralized that persists across scene loads.
//
#pragma once
#include <string>

// Timers that tick down and execute a specific noun/verb combo action when done.
struct GameTimer
{
    float secondsRemaining = 0.0f;
    std::string noun;
    std::string verb;
};

namespace GameTimers
{
    void Update(float deltaTime);
    void Set(const std::string& noun, const std::string& verb, float seconds);
}