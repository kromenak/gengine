#include "TextEncode.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

std::wstring TextEncode::Utf8ToUtf16(const std::string& utf8Str)
{
    // Early out in empty case.
    if(utf8Str.empty())
    {
        return L"";
    }

    // NOTE: This function is primarily meant for use on Windows where wstring is unavoidable in some cases.
    // NOTE: A non-Windows fallback is provided for completeness, but use is discouraged. Why are you using wstring anyways if not on Windows?

    #if defined(_WIN32)
    // Similar to vsnprintf, passing NULL allows us to calculate amount of bytes needed.
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], static_cast<int>(utf8Str.size()), NULL, 0);

    // Create wstring of necessary size and call again (not passing null) to actually populate it.
    std::wstring utf16Str(sizeNeeded, '\0');
    MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], static_cast<int>(utf8Str.size()), &utf16Str[0], sizeNeeded);
    return utf16Str;
    #else
    //TODO: Turns out this code is probably not correct. wchar_t is 32-bit on Linux and Mac, plus these functions depend on the system locale.
    //TODO: Fortunately, this UTF-16 conversion code is only needed on Windows right now!

    // Using a state object so that this implementation is thread-safe.
    std::mbstate_t state = std::mbstate_t();

    // Similar to vsnprintf, passing nullptr gives us the size needed for conversion.
    const char* utf8chars = utf8Str.c_str();
    std::size_t sizeNeeded = std::mbsrtowcs(nullptr, &utf8chars, 0, &state);

    // If mbsrtowcs fails due to an invalid multibyte character, it returns static_cast<std::size_t>(-1).
    // In this case, we can't do much - fail out!
    if(sizeNeeded == static_cast<std::size_t>(-1))
    {
        return L"";
    }

    // Convert to wstring and return.
    std::wstring wstr(sizeNeeded, '\0');
    std::mbsrtowcs(&wstr[0], &utf8chars, sizeNeeded, &state);
    return wstr;
    #endif
}

std::string TextEncode::Utf16ToUtf8(const std::wstring& utf16str)
{
    // Empty string early out.
    if(utf16str.empty())
    {
        return "";
    }

    // NOTE: This function is primarily meant for use on Windows where wstring is unavoidable in some cases.
    // NOTE: A non-Windows fallback is provided for completeness, but use is discouraged. Why are you using wstring anyways if not on Windows?

    #if defined(_WIN32)
    // WideCharToMultiByte uses integers, so really large strings can't be converted.
    if(utf16str.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        return "";
    }

    // Similar to vsnprintf, passing NULL allows us to calculate amount of bytes needed.
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &utf16str[0], static_cast<int>(utf16str.size()), NULL, 0, NULL, NULL);
    if(sizeNeeded <= 0)
    {
        return "";
    }

    // Create wstring of necessary size and call again (not passing null) to actually populate it.
    std::string utf8Str(sizeNeeded, '\0');
    WideCharToMultiByte(CP_UTF8, 0, &utf16str[0], static_cast<int>(utf16str.size()), &utf8Str[0], sizeNeeded, NULL, NULL);
    return utf8Str;
    #else
    //TODO: Turns out this code is probably not correct. wchar_t is 32-bit on Linux and Mac, plus these functions depend on the system locale.
    //TODO: Fortunately, this UTF-16 conversion code is only needed on Windows right now!

    // Using a state object so that this implementation is thread-safe.
    std::mbstate_t state = std::mbstate_t();

    // Similar to vsnprintf, passing nullptr gives us the size needed for conversion.
    const wchar_t* utf16chars = utf16str.c_str();
    std::size_t sizeNeeded = std::wcsrtombs(nullptr, &utf16chars, 0, &state);

    // If mbsrtowcs fails due to an invalid multibyte character, it returns static_cast<std::size_t>(-1).
    // In this case, we can't do much - fail out!
    if(sizeNeeded == static_cast<std::size_t>(-1))
    {
        return "";
    }

    // Convert to wstring and return.
    std::string str(sizeNeeded, '\0');
    std::wcsrtombs(&str[0], &utf16chars, sizeNeeded, &state);
    return str;
    #endif
}