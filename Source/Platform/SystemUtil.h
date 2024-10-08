//
// Clark Kromenaker
//
// Some cross-platform helper functions for querying the system.
//
#pragma once
#include <string>

#include "Platform.h"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <lmcons.h> // for ULEN define
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(HAVE_LIMITS_H)
#include <limits.h>
#endif

namespace SystemUtil
{
	// NOTE: Don't call this GetComputerName b/c Windows.h conflicts with that!
	inline std::string GetMachineName()
	{
        #if defined(PLATFORM_WINDOWS)
        {
            const DWORD kBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
            char computerName[kBufferSize];
            DWORD bufferSize = kBufferSize;
            if(!GetComputerName(computerName, &bufferSize))
            {
                std::cout << "Failed to get computer name!" << std::endl;
                return std::string("");
            }
            return std::string(computerName);
        }
        #elif defined(HAVE_UNISTD_H) && defined(HAVE_LIMITS_H)
        {
            char computerName[_POSIX_HOST_NAME_MAX];
            gethostname(computerName, _POSIX_HOST_NAME_MAX);
            return std::string(computerName);
        }
        #else
            #error "No implementation for SystemUtil::GetMachineName!"
        #endif
	}
	
	// NOTE: Don't call this GetUserName b/c Windows.h conflicts with that!
	inline std::string GetCurrentUserName()
	{
        #if defined(PLATFORM_WINDOWS)
        {
            const DWORD kBufferSize = UNLEN + 1;
            char userName[kBufferSize];
            DWORD bufferSize = kBufferSize;
            if(!GetUserName(userName, &bufferSize))
            {
                std::cout << "Failed to get user name!" << std::endl;
                return std::string("");
            }
            return std::string(userName);
        }
        #elif defined(HAVE_UNISTD_H) && defined(HAVE_LIMITS_H)
        {
            char userName[_POSIX_LOGIN_NAME_MAX];
            getlogin_r(userName, _POSIX_LOGIN_NAME_MAX);
            return std::string(userName);
        }
        #else
            #error "No implementation for SystemUtil::GetCurrentUserName!"
        #endif
	}

    inline void GetTime(uint16_t& year, uint16_t& month, uint16_t& dayOfWeek, uint16_t& day,
                        uint16_t& hour, uint16_t& minute, uint16_t& second, uint16_t& milliseconds)
    {
        #if defined(PLATFORM_WINDOWS)
        {
            SYSTEMTIME time;
            ::GetSystemTime(&time);
            year = time.wYear;
            month = time.wMonth;
            dayOfWeek = time.wDayOfWeek;
            day = time.wDay;
            hour = time.wHour;
            minute = time.wMinute;
            second = time.wSecond;
            milliseconds = time.wMilliseconds;
        }
        #else
        {
            year = 0;
            month = 0;
            dayOfWeek = 0;
            day = 0;
            hour = 0;
            minute = 0;
            second = 0;
            milliseconds = 0;
        }
        #endif
    }
}
