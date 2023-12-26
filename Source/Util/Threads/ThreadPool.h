//
// Clark Kromenaker
//
// A thread pool provides a generalized/simple way to run code on background threads.
// Just add a task and the next available thread will do the work.
//
#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

#include "ThreadUtil.h"

class ThreadedTaskQueue
{
public:
    ThreadedTaskQueue(int threadCount = 0);
    ~ThreadedTaskQueue();

    void AddThreads(int count);
    void Shutdown();

    void AddTask(const std::function<void()>& task, const std::function<void()>& callback = nullptr);
    void AddTask(const std::function<void(void*)>& task, void* context = nullptr, const std::function<void()>& callback = nullptr);

private:
    struct Task
    {
        // The task and task context.
        std::function<void(void*)> task;
        void* context = nullptr;

        // A callback for when the task completes.
        std::function<void()> callback;

        Task(std::function<void(void*)> task, void* context, std::function<void()> callback) :
            task(std::move(task)),
            context(context),
            callback(std::move(callback))
        {

        }
    };

    // Tasks waiting to be completed.
    std::queue<Task> mTasks;

    // If true, task queue is shutting down. Worker threads will exit.
    bool mShutdown = false;

    // Mutex/condition variable to deal with multi-threaded access to shared data.
    std::mutex mMutex;
    std::condition_variable mCondVar;

    // Threads spawned for this task queue.
    std::vector<std::thread> mThreads;

    void TaskThread();
};

class ThreadPool
{
public:
    static void Init(int threadCount);
    static void Shutdown();

    static void AddTask(const std::function<void()>& task, const std::function<void()>& callback = nullptr);
    static void AddTask(const std::function<void(void*)>& task, void* context = nullptr, const std::function<void()>& callback = nullptr);

private:
    // Just uses a threaded task queue internally.
    // The thread pool is really just a static instance of a task queue!
    static ThreadedTaskQueue sTaskQueue;
};