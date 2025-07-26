//
// Clark Kromenaker
//
// Loader tracks outstanding background tasks and performs them on background threads.
// Ideally, any loading work the game does that can go through the loader probably should!
//
// Keep in mind: when the loader is running, the game is not playable. A spinning loading cursor is displayed.
// So, only use it when some work needs to be done before the game can continue playing.
//
#pragma once
#include <functional>

#include "ThreadPool.h"
#include "Timers.h"

class Loader
{
public:
    static void Shutdown();

    static void Load(const std::function<void()>& loadFunc);
    static void DoAfterLoading(const std::function<void()>& callback);

    static void AddLoadingTask();
    static void RemoveLoadingTask();
    static bool IsLoading() { return sLoadingCount > 0; }

private:
    // Threads devoted to loading tasks.
    static ThreadedTaskQueue sLoadingTasks;

    // Number of loading tasks.
    static int sLoadingCount;

    // Callback for when loading finishes.
    static std::function<void()> sLoadingFinishedCallback;

    // Tracks how long loading takes and outputs some timing data to the log.
    static Stopwatch sLoadingStopwatch;

    static void OnLoadingFinished();
};
