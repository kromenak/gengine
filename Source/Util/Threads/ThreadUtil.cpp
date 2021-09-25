#include "ThreadUtil.h"

std::vector<std::function<void()>> ThreadUtil::sMainThreadFuncs;
std::mutex ThreadUtil::sMutex;

void ThreadUtil::RunOnMainThread(std::function<void()> func)
{
    if(func != nullptr)
    {
        sMutex.lock();
        sMainThreadFuncs.push_back(func);
        sMutex.unlock();
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