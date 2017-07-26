//
//  SDLLog.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
//

#include "SDLLog.h"
#include <cstdarg>

void SDLLog::Log(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    SDL_Log(fmt, list);
    va_end(list);
}
