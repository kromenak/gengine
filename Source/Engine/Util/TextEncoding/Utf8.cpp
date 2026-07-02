#include "TextEncode.h"

#include "Platform.h"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

void TextEncode::InitUtf8()
{
    #if defined(PLATFORM_WINDOWS)
    //TODO: On Windows 10, we could check/assert that we're using the ACP code page.

    // Allows the output console to display UTF-8 encoded text.
    SetConsoleOutputCP(CP_UTF8);
    #endif
}