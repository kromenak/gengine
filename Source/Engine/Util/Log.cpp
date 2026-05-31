#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <mutex>

namespace
{
    // Helper struct just ensures fclose gets called on the log file when application exits.
    struct LogFile
    {
        FILE* file = nullptr;

        ~LogFile()
        {
            if(file != nullptr)
            {
                fclose(file);
            }
        }
    };
    LogFile logFile;

    // A mutex for writing to the log. Ensures logging from multiple threads gives correct output.
    std::mutex logMutex;

    // A buffer to contain formatted text. Starts at 1KB in size, but will expand dynamically if needed.
    std::string formatBuffer(1024, '\0');

    // A mutex for writing to the format buffer.
    std::mutex formatMutex;
}

void Log(const char* message)
{
    std::lock_guard<std::mutex> lock(logMutex);

    // Open log file if not yet opened.
    if(logFile.file == nullptr)
    {
        logFile.file = fopen("GK3.log", "w");
    }

    // Log to standard output, adding a line break.
    printf("%s\n", message);

    // Log to file, adding a line break.
    fprintf(logFile.file, "%s\n", message);
}

void Logf(const char* format, ...)
{
    std::lock_guard<std::mutex> lock(formatMutex);

    // Generate formatted string in format buffer.
    va_list args;
    va_start(args, format);
    int writtenChars = vsnprintf(formatBuffer.data(), formatBuffer.length(), format, args);
    va_end(args);

    // If trying to write a message too big for the format buffer, try again after increasing format buffer size.
    // This should be rare, since the default format buffer size of 1KB is already pretty large!
    if(writtenChars >= formatBuffer.length())
    {
        formatBuffer.resize(writtenChars);
        vsnprintf(formatBuffer.data(), formatBuffer.length(), format, args);
    }

    // Log the message.
    Log(formatBuffer.c_str());
}
