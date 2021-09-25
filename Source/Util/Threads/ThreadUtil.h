//
// Clark Kromenaker
//
// Misc thread utilities.
//
#pragma once
#include <functional>
#include <mutex>
#include <vector>

class ThreadUtil
{
public:
    // A centralized way to allow threads to call back to the main thread.
    static void RunOnMainThread(std::function<void()> func);
    static void RunFunctionsOnMainThread();

private:
    static std::vector<std::function<void()>> sMainThreadFuncs;
    static std::mutex sMutex;
};