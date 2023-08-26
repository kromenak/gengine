#include "Profiler.h"

#include <cassert>
#include <iostream>

#include <SDL.h>

#include "Platform.h"

uint64_t Profiler::sFrameNumber = 0L;
std::vector<Sample> Profiler::sActiveSamples;

Stopwatch::Stopwatch()
{
    Reset();
}

void Stopwatch::Reset()
{
    mStartCounter = SDL_GetPerformanceCounter();
}

float Stopwatch::GetMilliseconds() const
{
    // Convert to milliseconds.
    return GetSeconds() * 1000.0f;
}

float Stopwatch::GetSeconds() const
{
    // Get counter delta since stopwatch started.
    uint64_t counter = SDL_GetPerformanceCounter();
    uint64_t count = counter - mStartCounter;

    // Convert to seconds.
    return (static_cast<float>(count) / static_cast<float>(SDL_GetPerformanceFrequency()));
}

Sample::Sample(const char* name) :
    mName(name)
{

}

Sample::~Sample()
{
    printf("[%s] %.2f ms\n", mName, mTimer.GetMilliseconds());
}

/*static*/ void Profiler::BeginFrame()
{
    sActiveSamples.clear();

    #if defined(ENV64)
    printf("===== Begin Frame %lu =====\n", sFrameNumber);
    #else
    printf("===== Begin Frame %llu =====\n", sFrameNumber);
    #endif
    BeginSample("Total");
}

/*static*/ void Profiler::EndFrame()
{
    // There should only be one sample active (from BeginFrame) at this point.
    // If not, there are mismatched begin/end sample calls somewhere.
    assert(sActiveSamples.size() == 1);

    // End overall frame sample.
    EndSample();
    #if defined(ENV64)
    printf("===== End Frame %lu =====\n", sFrameNumber);
    #else
    printf("===== End Frame %llu =====\n", sFrameNumber);
    #endif

    // Increment frame number at end of frame (if you do this at beginning, it just means there's no frame 0).
    sFrameNumber++;
}

/*static*/ void Profiler::BeginSample(const char* name)
{
    // Put sample in stack.
    sActiveSamples.emplace_back(name);
}

/*static*/ void Profiler::EndSample()
{
    // Pop the sample off the stack (prints sample info to log).
    sActiveSamples.pop_back();
}