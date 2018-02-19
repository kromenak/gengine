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
    
    inline void Trim(std::string& str)
    {
        // Find first non-space character index. If we can't (str is all spaces), just return.
        size_t first = str.find_first_not_of(' ');
        if(first == string::npos) { return; }
        
        // Find first non-space in the back.
        size_t last = str.find_last_not_of(' ');
        
        // Finally, trim off the front and back whitespace.
        str = str.substr(first, (last - first + 1));
    }
}
