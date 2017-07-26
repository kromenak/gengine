//
//  SDLLog.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//

#pragma once

#include "SDL/SDL.h"

class SDLLog
{
public:
    void Log(const char *fmt, ...);
    //void LogWarning(const char *fmt, ...);
    //void LogError(const char *fmt, ...);
};
