//
// Clark Kromenaker
//
// Helper for platform-abstraction. Provides a uniform/simplified way
// to detect what platform we're running on.
//
#pragma once

#if defined(__APPLE__)
	#include <TargetConditionals.h>
	#if defined(TARGET_OS_OSX)
		#define PLATFORM_MAC
	#else
		#error "Unknown Apple Platform"
	#endif
#elif defined(_WIN32)
	#define PLATFORM_WINDOWS
#else
	#error "Unknown Platform"
#endif
