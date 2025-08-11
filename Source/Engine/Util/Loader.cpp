#include <cstdio>

#include "Loader.h"

#include "ThreadPool.h"

// Loader uses a single background thread, for now.
ThreadedTaskQueue Loader::sLoadingTasks(1);

int Loader::sLoadingCount = 0;
std::function<void()> Loader::sLoadingFinishedCallback;
Stopwatch Loader::sLoadingStopwatch;

void Loader::Shutdown()
{
    sLoadingTasks.Shutdown();
}

void Loader::Load(const std::function<void()>& loadFunc)
{
    if(loadFunc != nullptr)
    {
        AddLoadingTask();
        sLoadingTasks.AddTask(loadFunc, []() {
            RemoveLoadingTask();
        });
    }
}

void Loader::DoAfterLoading(const std::function<void()>& callback)
{
    // Save callback.
    if(callback != nullptr)
    {
        sLoadingFinishedCallback = callback;
    }

    // If nothing is loading right now, loading must already be done.
    // So, do the callback right away!
    if(sLoadingCount == 0)
    {
        OnLoadingFinished();
    }
}

void Loader::AddLoadingTask()
{
    // If this is the first loading task added, reset the stopwatch.
    // We are now timing a new "batch" of loading tasks.
    if(sLoadingCount == 0)
    {
        printf("Starting load\n");
        sLoadingStopwatch.Reset();
    }

    ++sLoadingCount;
}

void Loader::RemoveLoadingTask()
{
    --sLoadingCount;

    // If no more active loading tasks, consider the current loading "batch" to be done.
    if(sLoadingCount == 0)
    {
        printf("Loading completed in %f seconds.\n", sLoadingStopwatch.GetSeconds());
        OnLoadingFinished();
    }
}

void Loader::OnLoadingFinished()
{
    auto callback = sLoadingFinishedCallback;
    sLoadingFinishedCallback = nullptr;
    if(callback != nullptr)
    {
        callback();
    }
}