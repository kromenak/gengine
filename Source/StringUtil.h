//
//  StringUtil.h
//  GEngine
//
//  Created by Clark Kromenaker on 12/30/17.
//
#pragma once
#include <string>
#include <algorithm>
#include <cctype>

namespace StringUtil
{
    inline void ToUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }
}
