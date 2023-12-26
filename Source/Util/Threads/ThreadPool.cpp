#include "ThreadPool.h"

#include <thread>

ThreadedTaskQueue::ThreadedTaskQueue(int threadCount)
{
    AddThreads(threadCount);
}

ThreadedTaskQueue::~ThreadedTaskQueue()
{
    Shutdown();
}

void ThreadedTaskQueue::AddThreads(int count)
{
    for(int i = 0; i < count; i++)
    {
        mThreads.emplace_back([this] { TaskThread(); });
    }
}

void ThreadedTaskQueue::Shutdown()
{
    mMutex.lock();
    mShutdown = true;
    mMutex.unlock();

    mCondVar.notify_all();

    for(auto& thread : mThreads)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }
}

void ThreadedTaskQueue::AddTask(const std::function<void()>& task, const std::function<void()>& callback)
{
    // Wrap the task in a void(void*) lambda so it can be stored in the list successfully.
    AddTask([task](void* arg) {
        task();
    }, nullptr, callback);
}

void ThreadedTaskQueue::AddTask(const std::function<void(void*)>& task, void* context, const std::function<void()>& callback)
{
    if(task != nullptr)
    {
        mMutex.lock();
        mTasks.emplace(task, context, callback);
        mMutex.unlock();

        mCondVar.notify_one();
    }
}

void ThreadedTaskQueue::TaskThread()
{
    while(true)
    {
        // Lock mutex to check task list.
        std::unique_lock<std::mutex> lock(mMutex);

        // If task queue is empty (and not shutting down), wait.
        while(mTasks.empty() && !mShutdown)
        {
            mCondVar.wait(lock);
        }

        // If shutting down, just return.
        if(mShutdown) { return; }

        // Grab a task off the list.
        Task task = mTasks.front();
        mTasks.pop();

        // Done with task list for now.
        lock.unlock();

        // Do the task.
        task.task(task.context);

        // After the task is done, run callback on main thread.
        ThreadUtil::RunOnMainThread(task.callback);
    }
}

ThreadedTaskQueue ThreadPool::sTaskQueue;

void ThreadPool::Init(int threadCount)
{
    sTaskQueue.AddThreads(threadCount);
}

void ThreadPool::Shutdown()
{
    sTaskQueue.Shutdown();
}

void ThreadPool::AddTask(const std::function<void()>& task, const std::function<void()>& callback)
{
    sTaskQueue.AddTask(task, callback);
}

void ThreadPool::AddTask(const std::function<void(void*)>& task, void* context, const std::function<void()>& callback)
{
    sTaskQueue.AddTask(task, context, callback);
}