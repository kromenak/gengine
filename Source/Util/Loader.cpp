#include "Loader.h"

#include "ThreadPool.h"

// Loader uses a single background thread, for now.
ThreadedTaskQueue Loader::sLoadingTasks(1);

int Loader::sLoadingCount = 0;
std::function<void()> Loader::sLoadingFinishedCallback;

void Loader::Shutdown()
{
    sLoadingTasks.Shutdown();
}

void Loader::Load(std::function<void()> loadFunc)
{
    if(loadFunc != nullptr)
    {
        sLoadingCount++;
        sLoadingTasks.AddTask(loadFunc, []() {
            sLoadingCount--;
            if(sLoadingCount == 0)
            {
                OnLoadingFinished();
            }
        });
    }
}

void Loader::DoAfterLoading(std::function<void()> callback)
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

void Loader::OnLoadingFinished()
{
    auto callback = sLoadingFinishedCallback;
    sLoadingFinishedCallback = nullptr;
    if(callback != nullptr)
    {
        callback();
    }
}