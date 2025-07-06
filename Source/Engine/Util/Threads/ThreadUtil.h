//
// Clark Kromenaker
//
// Misc thread utilities.
//
#pragma once
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class ThreadUtil
{
public:
    // Initialize - should be called on main thread!
    static void Init();

    // Are we currently on main thread?
    static bool OnMainThread();

    // A centralized way to allow threads to call back to the main thread.
    static void RunOnMainThread(const std::function<void()>& func);
    static void RunFunctionsOnMainThread();

private:
    // The main thread's ID. Used to determine if functions are running on main thread.
    static std::thread::id sMainThreadId;

    // Functions that we want to run on main thread.
    static std::vector<std::function<void()>> sMainThreadFuncs;
    static std::mutex sMutex;
};
