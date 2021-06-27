#include "Profiler.h"

#include <cassert>
#include <iostream>

#include <SDL2/SDL.h>

#include "GMath.h"

uint64_t Profiler::sFrameNumber = 0L;
std::vector<Profiler::Sample> Profiler::sActiveSamples;

/*static*/ void Profiler::BeginFrame()
{
    sActiveSamples.clear();

    printf("===== Begin Frame %lli =====\n", sFrameNumber);
    BeginSample("Total");
}

/*static*/ void Profiler::EndFrame()
{
    // There should only be one sample active (from BeginFrame) at this point.
    // If not, there are mismatched begin/end sample calls somewhere.
    assert(sActiveSamples.size() == 1);

    // End overall frame sample.
    EndSample();
    printf("===== End Frame %lli =====\n", sFrameNumber);

    // Increment frame number at end of frame (if you do this at beginning, it just means there's no frame 0).
    sFrameNumber++;
}

/*static*/ void Profiler::BeginSample(const char* name)
{
    // Put sample in stack.
    sActiveSamples.emplace_back();

    // Record name and start time.
    sActiveSamples.back().name = name;
    sActiveSamples.back().startCounter = SDL_GetPerformanceCounter();
}

/*static*/ void Profiler::EndSample()
{
    // Calculate how many milliseconds have passed since the sample was started.
    uint64_t endFrameCounter = SDL_GetPerformanceCounter();
    uint64_t count = endFrameCounter - sActiveSamples.back().startCounter;
    double milliseconds = (static_cast<double>(count) / SDL_GetPerformanceFrequency()) * 1000.0;
    
    // Print out the stat.
    printf("%s: %.2f ms\n", sActiveSamples.back().name, milliseconds);
    
    // Pop the sample off the stack.
    sActiveSamples.pop_back();
}