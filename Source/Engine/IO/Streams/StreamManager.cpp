#include "StreamManager.h"

#include <fstream>
#include <mutex>
#include <unordered_map>

namespace
{
    // A null stream that's used when a valid stream can't be returned.
    // This stream is always in an error state, and reads/writes are effectively no-ops.
    std::iostream nullStream(nullptr);

    // Tracks internal state of an i/o stream.
    struct StreamTracker
    {
        // The name of the stream.
        std::string streamName;

        // The stream itself.
        std::unique_ptr<std::iostream> stream = nullptr;

        // A mutex for safe multi-threaded reading/writing from stream.
        std::mutex streamMutex;

        // How many references exist to this stream?
        // A stream is retained until its reference count becomes zero.
        int referenceCount = 0;
    };

    // Maps a name to a stream. This is how we ensure we only have one stream per file, for example.
    std::unordered_map<std::string, std::unique_ptr<StreamTracker>> streamTrackers;

    // Guards against concurrent access to the stream tracker map and stream tracker instances.
    std::mutex streamTrackerMutex;
}

StreamHandle StreamManager::TakeFileStream(const std::string& filePath, bool truncate)
{
    // We must lock the map mutex any time something may be added or removed from the open streams map.
    std::lock_guard<std::mutex> lock(streamTrackerMutex);

    // Find a stream tracker for this file path.
    std::unique_ptr<StreamTracker>& streamTracker = streamTrackers[filePath];

    // If null, none exists and we need to create one.
    if(streamTracker == nullptr)
    {
        streamTracker = std::make_unique<StreamTracker>();

        // Save file path as stream name.
        streamTracker->streamName = filePath;

        // std::fstream fails to open a file that doesn't exist. To ensure it succeeds, briefly create an ofstream so the file exists (in append mode to avoid deleting data).
        // (Assuming caller knows what they're doing and wants the file to exist. Use File::Exists first if not.)
        {
            std::ofstream(filePath.c_str(), std::ios::app);
        }

        // Create with both in/out capabilities.
        // "binary" flag is misleading - it allows writing text too. Main difference is it doesn't convert \n to \r\n on Windows.
        std::ios::openmode openMode = std::ios::in | std::ios::out | std::ios::binary;
        if(truncate)
        {
            openMode |= std::ios::trunc;
        }
        else
        {
            openMode |= std::ios::app;
        }

        // Create the stream.
        streamTracker->stream = std::make_unique<std::fstream>(filePath.c_str(), openMode);
    }

    // Increment reference counter and return handle.
    ++streamTracker->referenceCount;
    return streamTracker.get();
}

void StreamManager::ReturnStream(StreamHandle handle)
{
    // Ignore null streams.
    if(handle == nullptr) { return; }

    // Lock the mutex because we may erase the stream tracker from the map, if null.
    // And also because we're modifying the reference count!
    std::lock_guard<std::mutex> lock(streamTrackerMutex);

    // Decrement reference count.
    StreamTracker* streamTracker = static_cast<StreamTracker*>(handle);
    --streamTracker->referenceCount;

    // If no more references, "close" the stream by deleting it.
    if(streamTracker->referenceCount <= 0)
    {
        auto it = streamTrackers.find(streamTracker->streamName);
        if(it != streamTrackers.end())
        {
            streamTrackers.erase(it);
        }
    }
}

std::iostream& StreamManager::GetStream(StreamHandle handle)
{
    // Ignore null streams.
    if(handle == nullptr) { return nullStream; }

    // Purposely not guarding against misuse here. Handle shouldn't be null, and it should be valid (e.g. ref count is at least one).
    StreamTracker* streamTracker = static_cast<StreamTracker*>(handle);
    return *streamTracker->stream;
}

std::iostream& StreamManager::LockStream(StreamHandle handle)
{
    // Ignore null streams.
    if(handle == nullptr) { return nullStream; }

    // Lock the stream mutex.
    StreamTracker* streamTracker = static_cast<StreamTracker*>(handle);
    streamTracker->streamMutex.lock();

    // Once locked, get the stream and return it.
    return GetStream(handle);
}

void StreamManager::UnlockStream(StreamHandle handle)
{
    // Ignore null streams.
    if(handle == nullptr) { return; }

    // Unlock the mutex.
    StreamTracker* streamTracker = static_cast<StreamTracker*>(handle);
    streamTracker->streamMutex.unlock();
}