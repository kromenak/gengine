//
//  Clark Kromenaker
//
// "Add ons" to std::string.
// A bunch of handy utility functions that you wish were just built-in!
//
#pragma once
#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace StringUtil
{
    inline void ToUpper(std::string& str)
    {
        for(char& c : str)
        {
            c = static_cast<char>(std::toupper(c));
        }
    }

    inline std::string ToUpperCopy(std::string str)
    {
        for(char& c : str)
        {
            c = static_cast<char>(std::toupper(c));
        }
        return str;
    }

    inline void ToLower(std::string& str)
    {
        for(char& c : str)
        {
            c = static_cast<char>(std::tolower(c));
        }
    }

    inline std::string ToLowerCopy(std::string str)
    {
        for(char& c : str)
        {
            c = static_cast<char>(std::tolower(c));
        }
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

    inline void TrimWhitespace(std::string& str)
    {
        while(!str.empty() && (str.front() == ' ' || str.front() == '\t' || str.back() == ' ' || str.back() == '\t'))
        {
            Trim(str, ' ');
            Trim(str, '\t');
        }
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

    inline std::vector<std::string> Split(const std::string& str, char delim, bool removeEmpty = false)
    {
        std::stringstream ss(str);
        std::vector<std::string> tokens;
        std::string item;
        while(getline(ss, item, delim))
        {
            if(removeEmpty && item.empty()) { continue; }
            tokens.push_back(item);
        }
        return tokens;
    }

    inline void RemoveQuotes(std::string& str)
    {
        // Remove any whitespace on left/right.
        Trim(str);
        if(str.empty()) { return; }

        // Check if first char is a quote, and remove if so.
        if(str[0] == '"')
        {
            str.erase(str.begin());
        }
        if(str.empty()) { return; }

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
            // Get rid of anything after a comment.
            TrimComment(str);

            // "getline" can sometimes leave some unwanted chars on the end of the line, so let's get rid of those.
            // "getline" discards \n on end of lines, but leaves Windows line breaks (\r) - get rid of them!
            // For the last line in a stream, "getline" may add an extra \0 - don't need it, and causes string append bugs.
            while(!str.empty() && (str.back() == '\r' || str.back() == '\0'))
            {
                str.pop_back();
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

    inline bool EndsWith(const std::string& str, const std::string& endsWith)
    {
        if(endsWith.size() > str.size()) { return false; }
        return std::equal(endsWith.rbegin(), endsWith.rend(), str.rbegin());
    }

    inline bool EndsWithIgnoreCase(const std::string& str, const std::string& endsWith)
    {
        if(endsWith.size() > str.size()) { return false; }
        return std::equal(endsWith.rbegin(), endsWith.rend(), str.rbegin(), iequal());
    }

    inline size_t Find(const std::string& str, const std::string& toFind, size_t pos = 0)
    {
        if(str.size() - pos < toFind.size()) { return std::string::npos; }
        return str.find(toFind, pos);
    }

    inline size_t FindIgnoreCase(const std::string& str, const std::string& toFind, size_t pos = 0)
    {
        if(str.size() - pos < toFind.size()) { return std::string::npos; }
        auto it = std::search(str.begin() + pos, str.end(), toFind.begin(), toFind.end(), iequal());
        if(it != str.end()) { return it - str.begin(); }
        return std::string::npos;
    }

    inline bool ToBool(const std::string& str)
    {
        // True is "on" or "yes" or "true".
        // Anything is false.
        // TODO: If the value is a number, should we interpret 0=FALSE, ANY OTHER NUMBER=TRUE?
        return EqualsIgnoreCase(str, "on") || EqualsIgnoreCase(str, "yes") || EqualsIgnoreCase(str, "true");
    }

    inline int ToInt(const std::string& str)
    {
        return atoi(str.c_str());
    }

    inline float ToFloat(const std::string& str)
    {
        return static_cast<float>(atof(str.c_str()));
    }

	template<typename ... Args>
	inline std::string Format(const char* format, Args ... args)
	{
		// Calling snprintf with nullptr & 0 buff_size let's you determine the expected size of the result.
		// Per: https://en.cppreference.com/w/cpp/io/c/fprintf
		// +1 for the \0 null terminator.
		size_t size = snprintf(nullptr, 0, format, args ...) + 1;

		// Allocate a buffer to hold the formatted text.
		// Using unique_ptr for auto-delete on return or exception.
		std::unique_ptr<char[]> buf(new char[size]);

		// Actually put the formatted string in the buffer "for real".
		snprintf(buf.get(), size, format, args ...);

		// Create a string from the buffer (-1 b/c we don't need the \0 for the string).
		return std::string(buf.get(), buf.get() + size - 1);
	}

    inline unsigned long HashCaseInsensitive(const char* str)
    {
        // DJB2 hash, XOR variant, case-insensitive.
        // Taken from http://www.cse.yorku.ca/~oz/hash.html
        unsigned long hash = 5381;
        int c;
        while((c = *str++) != 0)
        {
            hash = ((hash << 5) + hash) ^ std::toupper(c); /* hash * 33 XOR c */
        }
        return hash;
    }

    inline unsigned long HashCaseInsensitive(const std::string& str)
    {
        return HashCaseInsensitive(str.c_str());
    }

    // Helper structs for using std collections with case-insensitive comparisons/hashing.
    struct CaseInsensitiveCompare
    {
        bool operator() (const std::string& lhs, const std::string& rhs) const
        {
            return StringUtil::EqualsIgnoreCase(lhs, rhs);
        }
    };
    struct CaseInsensitiveHash
    {
        std::size_t operator() (const std::string& str) const
        {
            return StringUtil::HashCaseInsensitive(str.c_str());
        }
    };
}

namespace std
{
    // Type alias for case-insensitive unordered map with string key.
    template <typename T>
    using string_map_ci = std::unordered_map<std::string, T,
                                             StringUtil::CaseInsensitiveHash,
                                             StringUtil::CaseInsensitiveCompare>;

    // Type alias for case-insensitive unordered map.
    // This version is meant primarily to allow easily swapping from unordered_map.
    // Though it lets you specifiy a key template type, the type must be std::string for it to compile!
    template <typename T, typename U>
    using unordered_map_ci = std::unordered_map<T, U,
                                             StringUtil::CaseInsensitiveHash,
                                             StringUtil::CaseInsensitiveCompare>;

    using string_set_ci = std::unordered_set<std::string,
                                             StringUtil::CaseInsensitiveHash,
                                             StringUtil::CaseInsensitiveCompare>;
}
