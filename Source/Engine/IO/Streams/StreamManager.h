//
// Clark Kromenaker
//
// In many cases, it's perfectly fine to create and use local streams.
//
// But in some complex scenarios, we could have multiple systems that want to write to a single file.
// Or worse, multiple threads trying to write to a file at the same time.
//
// This manager provides a wya to obtain shared file streams in a thread-safe way.
// Streams can also be locked to ensure mutual read/write access for a limited time.
//
#pragma once
#include <iostream>
#include <string>

// An opaque handle to a stream.
typedef void* StreamHandle;

namespace StreamManager
{
    // Take/return a stream handle.
    // Reference counting happens behind the scenes. The stream is only closed when no more references exist.
    StreamHandle TakeFileStream(const std::string& filePath, bool truncate = false);
    void ReturnStream(StreamHandle handle);

    // Allows obtaining the iostream associated with a handle *without* worrying about locking the stream.
    // Only use this if you know you won't be writing the file from multiple threads.
    std::iostream& GetStream(StreamHandle handle);

    // Lock and unlock a stream. Be sure to always unlock after locking!
    std::iostream& LockStream(StreamHandle handle);
    void UnlockStream(StreamHandle handle);
}
