//
// Clark Kromenaker
//
// Helper for platform-abstraction. Provides a uniform/simplified way
// to detect what platform we're running on.
//
#pragma once
#include "BuildEnv.h" // Contains info on what headers are available on this platform

#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_OSX)
        #define PLATFORM_MAC
        #define ENV64
    #else
        #error "Unknown Apple Platform"
    #endif
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS
    #define ENV32
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define ENV64
#else
    #error "Unknown Platform"
#endif