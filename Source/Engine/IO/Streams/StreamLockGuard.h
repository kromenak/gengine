//
// Clark Kromenaker
//
// Provides access to an exclusively locked stream.
//
// An RAII helper, similar to std::lock_guard.
// Meant to be used in small scopes to ensure exclusive read/write access to a file.
//
#pragma once
#include "StreamManager.h"

struct StreamLockGuard
{
    explicit StreamLockGuard(StreamHandle handle) :
        mStreamHandle(handle),
        stream(StreamManager::LockStream(mStreamHandle))
    {

    }

    ~StreamLockGuard()
    {
        StreamManager::UnlockStream(mStreamHandle);
    }

    // Prevent copying.
    StreamLockGuard(const StreamLockGuard&) = delete;
    StreamLockGuard& operator=(const StreamLockGuard&) = delete;

private:
    // The handle to the stream.
    StreamHandle mStreamHandle = nullptr;

public:
    // The locked and ready to use stream. Public for easy access.
    std::iostream& stream;
};
