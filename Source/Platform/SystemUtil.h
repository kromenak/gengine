//
// Clark Kromenaker
//
// Some cross-platform helper functions for querying the system.
//
#pragma once
#include <climits>
#include <ctime>
#include <string>

#include "Platform.h"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <lmcons.h> // for ULEN define
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>

// Linux defines HOST_NAME_MAX, while macOS (and perhaps others) doesn't.
// _POSIX_HOST_NAME_MAX should be defined, so use that if we can, or fall back on a default.
#if !defined(HOST_NAME_MAX) && defined(_POSIX_HOST_NAME_MAX)
    #define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#elif !defined(HOST_NAME_MAX)
    #define HOST_NAME_MAX 255 // based on _POSIX_HOST_NAME_MAX on macOS
#endif

// Same idea for LOGIN_NAME_MAX.
#if !defined(LOGIN_NAME_MAX) && defined(_POSIX_LOGIN_NAME_MAX)
    #define LOGIN_NAME_MAX _POSIX_LOGIN_NAME_MAX
#elif !defined(LOGIN_NAME_MAX)
    #define LOGIN_NAME_MAX 9 // based on _POSIX_LOGIN_NAME_MAX on macOS
#endif
#endif

namespace SystemUtil
{
    inline std::string GetMachineName()
    {
        #if defined(PLATFORM_WINDOWS)
        {
            const DWORD kBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
            char computerName[kBufferSize] = { 0 };
            DWORD bufferSize = kBufferSize;
            if(!GetComputerName(computerName, &bufferSize))
            {
                printf("Failed to get machine name.\n");
                return std::string();
            }
            return std::string(computerName);
        }
        #elif defined(HAVE_UNISTD_H)
        {
            char computerName[HOST_NAME_MAX + 1] = { 0 };
            int res = gethostname(computerName, HOST_NAME_MAX);
            if(res != 0)
            {
                printf("Failed to get machine name.\n");
                return std::string();
            }
            return std::string(computerName);
        }
        #else
            #error "No implementation for SystemUtil::GetMachineName!"
        #endif
    }

    inline std::string GetCurrentUserName()
    {
        #if defined(PLATFORM_WINDOWS)
        {
            const DWORD kBufferSize = UNLEN + 1;
            char userName[kBufferSize];
            DWORD bufferSize = kBufferSize;
            if(!GetUserName(userName, &bufferSize))
            {
                printf("Failed to get user name.\n");
                return std::string();
            }
            return std::string(userName);
        }
        #elif defined(HAVE_UNISTD_H)
        {
            char userName[LOGIN_NAME_MAX + 1] = { 0 };
            int res = getlogin_r(userName, LOGIN_NAME_MAX);
            if(res != 0)
            {
                printf("Failed to get user name.\n");
                return std::string();
            }
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
            month = time.wMonth;                // 1-12 range
            dayOfWeek = time.wDayOfWeek;        // 0-6 range (0 is Sunday)
            day = time.wDay;                    // 1-31 range
            hour = time.wHour;                  // 0-23 range
            minute = time.wMinute;              // 0-59 range
            second = time.wSecond;              // 0-59 range
            milliseconds = time.wMilliseconds;  // 0-999 range
        }
        #else
        {
            time_t time = std::time(nullptr);
            std::tm* localTime = std::localtime(&time);
            if(localTime != nullptr)
            {
                year = 1900 + localTime->tm_year;   // years since 1900
                month = localTime->tm_mon + 1;      // +1 for 1-12 range
                dayOfWeek = localTime->tm_wday;     // 0-6 range (0 is Sunday)
                day = localTime->tm_mday;           // 1-31 range
                hour = localTime->tm_hour;          // 0-23 range
                minute = localTime->tm_min;         // 0-59 range
                second = localTime->tm_sec;         // 0-60 range
                milliseconds = 0;                   // not supported
            }
            else
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
        }
        #endif
    }
}