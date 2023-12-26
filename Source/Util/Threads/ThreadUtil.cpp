#include "ThreadUtil.h"

std::thread::id ThreadUtil::sMainThreadId;

std::vector<std::function<void()>> ThreadUtil::sMainThreadFuncs;
std::mutex ThreadUtil::sMutex;

void ThreadUtil::Init()
{
    // Save current thread ID as the main thread's ID.
    sMainThreadId = std::this_thread::get_id();
}

bool ThreadUtil::OnMainThread()
{
    return sMainThreadId == std::this_thread::get_id();
}

void ThreadUtil::RunOnMainThread(const std::function<void()>& func)
{
    if(func != nullptr)
    {
        if(OnMainThread())
        {
            func();
        }
        else
        {
            sMutex.lock();
            sMainThreadFuncs.push_back(func);
            sMutex.unlock();
        }
    }
}

void ThreadUtil::RunFunctionsOnMainThread()
{
    sMutex.lock();
    for(auto& func : sMainThreadFuncs)
    {
        func();
    }
    sMainThreadFuncs.clear();
    sMutex.unlock();
}