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
    
    inline std::string ToUpperCopy(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    
    inline void ToLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }
    
    inline std::string ToLowerCopy(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    inline void Trim(std::string& str, char trimChar)
    {
        // Find first non-space character index.
        // If we can't (str is all spaces), clear and return it.
        size_t first = str.find_first_not_of(trimChar);
        if(first == std::string::npos)
        {
            str.clear();
            return;
        }
        
        // Find first non-space in the back.
        size_t last = str.find_last_not_of(trimChar);
        
        // Trim off the front and back whitespace.
        str = str.substr(first, (last - first + 1));
    }
    
    inline void Trim(std::string& str)
    {
        Trim(str, ' ');
    }
    
    inline void RemoveQuotes(std::string& str)
    {
        // Remove any whitespace on left/right.
        Trim(str);
        if(str.size() == 0) { return; }
        
        // Check if first char is a quote, and remove if so.
        if(str[0] == '"')
        {
            str.erase(str.begin());
        }
        if(str.size() == 0) { return; }
        
        // Check if last char is a quote, and remove if so.
        if(str[str.size() - 1] == '"')
        {
            str.erase(str.end() - 1);
        }
    }
    
    // Struct that encapsulates a case-insensitive character comparison.
    struct iequal
    {
        bool operator()(int c1, int c2) const
        {
            return std::toupper(c1) == std::toupper(c2);
        }
    };
    
    inline bool EqualsIgnoreCase(const std::string& str1, const std::string& str2)
    {
        return std::equal(str1.begin(), str1.end(), str2.begin(), iequal());
    }
}
