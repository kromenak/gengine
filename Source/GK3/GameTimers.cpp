#include "GameTimers.h"

#include <vector>

#include "ActionManager.h"
#include "PersistState.h"

namespace
{
    // Timers that tick down and execute a specific noun/verb combo action when done.
    struct GameTimer
    {
        float secondsRemaining = 0.0f;
        std::string noun;
        std::string verb;

        void OnPersist(PersistState& ps)
        {
            ps.Xfer(PERSIST_VAR(secondsRemaining));
            ps.Xfer(PERSIST_VAR(noun));
            ps.Xfer(PERSIST_VAR(verb));
        }
    };

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
            gActionManager.ExecuteBackgroundAction(mGameTimers[i].noun, mGameTimers[i].verb);
            mGameTimers.erase(mGameTimers.begin() + i);
        }
    }
}

void GameTimers::Set(const std::string& noun, const std::string& verb, float seconds)
{
    // If seconds are zero or less (this does happen from time to time), just execute action immediately and return.
    if(seconds <= 0.0f && !gActionManager.IsActionPlaying())
    {
        gActionManager.ExecuteBackgroundAction(noun, verb);
        return;
    }

    // Otherwise, add to tracked timers.
    mGameTimers.emplace_back();
    mGameTimers.back().secondsRemaining = seconds;
    mGameTimers.back().noun = noun;
    mGameTimers.back().verb = verb;
}

void GameTimers::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mGameTimers));
}