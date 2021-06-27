#pragma once
#include <cstdint>
#include <vector>

class Profiler;
class Sampler;

//#define PROFILER_ENABLED

#if defined(PROFILER_ENABLED)
    #define PROFILER_BEGIN_FRAME() Profiler::BeginFrame();
    #define PROFILER_END_FRAME() Profiler::EndFrame();
    #define PROFILER_BEGIN_SAMPLE(x) Profiler::BeginSample(x);
    #define PROFILER_END_SAMPLE() Profiler::EndSample();
    #define PROFILER_SAMPLER(x) Sampler x(#x);
#else
    #define PROFILER_BEGIN_FRAME()
    #define PROFILER_END_FRAME()
    #define PROFILER_BEGIN_SAMPLE(x)
    #define PROFILER_END_SAMPLE()
    #define PROFILER_SAMPLER(x)
#endif

class Profiler
{
public:
    static void BeginFrame();
    static void EndFrame();

    static void BeginSample(const char* name);
    static void EndSample();

private:
    // Counts what frame we're on.
    static uint64_t sFrameNumber;

    // A "sample" is a part of the code we are timing.
    struct Sample
    {
        const char* name;
        uint64_t startCounter = 0L;
    };
    static std::vector<Sample> sActiveSamples;
};

// Small class that just handles calling BeginSample/EndSample.
// Just create at start of function and you are set.
class Sampler
{
public:
    Sampler(const char* name) { PROFILER_BEGIN_SAMPLE(name); }
    ~Sampler() { PROFILER_END_SAMPLE(); }
};