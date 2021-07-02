//
//  StringUtil.h
//
//  Clark Kromenaker
//
// "Add ons" to std::string.
// A bunch of handy utility functions that you wish were just built-in!
//
#pragma once
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

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
	
	inline void TrimComment(std::string& str)
	{
		// Trims any comment from trailing part of string.
		size_t found = str.find("//");
		if(found != std::string::npos)
		{
			str.erase(found);
		}
	}
	
	inline void RemoveAll(std::string& str, char remove)
	{
		str.erase(std::remove(str.begin(), str.end(), remove), str.end());
	}
    
    inline std::vector<std::string> Split(const std::string& str, char delim)
    {
        std::stringstream ss(str);
        std::vector<std::string> tokens;
        std::string item;
        while(getline(ss, item, delim))
        {
            tokens.push_back(item);
        }
        return tokens;
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
    
    inline bool GetLineSanitized(std::istream& is, std::string& str)
    {
        if(std::getline(is, str))
        {
            // "getline" reads up to the '\n' character in a file, and "eats" the '\n' too.
            // But Windows line breaks might include the '\r' character too, like "\r\n".
            // To deal with this semi-elegantly, we'll search for and remove the '\r' here.
            if(!str.empty() && str[str.length() - 1] == '\r')
            {
                str.resize(str.length() - 1);
            }
            
            // Trim the line of any whitespaces and tabs.
            Trim(str);
            Trim(str, '\t');
            return true;
        }
        return false;
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
        if(str1.size() != str2.size()) { return false; }
        return std::equal(str1.begin(), str1.end(), str2.begin(), iequal());
    }

    inline bool StartsWith(const std::string& str, const std::string& startsWith)
    {
        if(str.size() < startsWith.size()) { return false; }

        // Using "rfind" instead of "find" (with a start index of 0) ensures that only the first position is checked.
        // "find" could be used (check that first instance is at index 0), but it would search the entire string possibly.
        return str.rfind(startsWith, 0) == 0;
    }

    inline bool StartsWithIgnoreCase(const std::string& str, const std::string& startsWith)
    {
        if(str.size() < startsWith.size()) { return false; }

        // I believe if we only search from the beginning up to the size of the search string, it'll only
        // return a result IF the string starts with the search string.
        auto searchEndIt = str.begin() + startsWith.size();
        return std::search(str.begin(), searchEndIt, startsWith.begin(), startsWith.end(), iequal()) != searchEndIt;
    }

    inline bool Contains(const std::string& str, const std::string& contains)
    {
        if(str.size() < contains.size()) { return false; }
        return str.find(contains) != std::string::npos;
    }

    inline bool ContainsIgnoreCase(const std::string& str, const std::string& contains)
    {
        if(str.size() < contains.size()) { return false; }
        return std::search(str.begin(), str.end(), contains.begin(), contains.end(), iequal()) != str.end();
    }

    inline bool ToBool(const std::string& str)
    {
        // If the string is "yes" or "true", we'll say it converts to "true".
        // The values "no" or "false" would convert to false...but false will also be our default return value.
        // So, anything other than "yes" or "true" returns false.
        return EqualsIgnoreCase(str, "on") || EqualsIgnoreCase(str, "yes") || EqualsIgnoreCase(str, "true");
    }
    
    inline int ToInt(const std::string& str)
    {
        return atoi(str.c_str());
    }
    
    inline float ToFloat(const std::string& str)
    {
        return (float)atof(str.c_str());
    }

	template<typename ... Args>
	inline std::string Format(const std::string& format, Args ... args)
	{
		// Calling snprintf with nullptr & 0 buff_size let's you determine the expected size of the result.
		// Per: https://en.cppreference.com/w/cpp/io/c/fprintf
		// +1 for the \0 null terminator.
		size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
		
		// Allocate a buffer to hold the formatted text.
		// Using unique_ptr for auto-delete on return or exception.
		std::unique_ptr<char[]> buf(new char[size]);
		
		// Actually put the formatted string in the buffer "for real".
		snprintf(buf.get(), size, format.c_str(), args ...);
		
		// Create a string from the buffer (-1 b/c we don't need the \0 for the string).
		return std::string(buf.get(), buf.get() + size - 1);
	}
}
