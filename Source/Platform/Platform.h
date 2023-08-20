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
	#else
		#error "Unknown Apple Platform"
	#endif
#elif defined(_WIN32)
	#define PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#else
	#error "Unknown Platform"
#endif

// A define for all platforms that are expected to have posix headers.
#if defined(PLATFORM_MAC) || defined(PLATFORM_LINUX)
    #define PLATFORM_POSIX
#endif