#include "GameTimers.h"

#include <vector>

#include "ActionManager.h"

namespace
{
    // Timers that are active.
    std::vector<GameTimer> mGameTimers;
}

void GameTimers::Update(float deltaTime)
{
    // Decrement any game timers.
    for(int i = mGameTimers.size() - 1; i >= 0; --i)
    {
        mGameTimers[i].secondsRemaining -= deltaTime;

        // If another action is already playing, we wait until it is finished before executing this one (and removing it from the list).
        if(mGameTimers[i].secondsRemaining <= 0.0f && !gActionManager.IsActionPlaying())
        {
            gActionManager.ExecuteAction(mGameTimers[i].noun, mGameTimers[i].verb);
            mGameTimers.erase(mGameTimers.begin() + i);
        }
    }
}

void GameTimers::Set(const std::string& noun, const std::string& verb, float seconds)
{
    mGameTimers.emplace_back();
    mGameTimers.back().secondsRemaining = seconds;
    mGameTimers.back().noun = noun;
    mGameTimers.back().verb = verb;
}
