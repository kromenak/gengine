#pragma once
#include <string>

namespace MathStringUtil
{
    inline std::string TrimToNumbersOnly(const std::string& string)
    {
        // Input string could be anything, probably a single number or a set of numbers. May have leading/trailing cruft, like braces.
        // The goal is to get rid of all non-numeric leading/trailing characters, leaving just number data.
        // Example input: "sjdd{ -20, 30, 200, 10 }]]]dsad"
        // Example output: "-20, 30, 200, 10"

        // Trim leading non-numeric values.
        // This is usually just '{', but could be anything.
        int firstNumIndex = -1;
        for(int i = 0; i < string.length(); ++i)
        {
            // Also need to check '-' for symbol preceding negative numbers.
            // Also also need to check for '.' symbol for shorthand like ".42"
            if(std::isdigit(string[i]) || string[i] == '-' || string[i] == '.')
            {
                firstNumIndex = i;
                break;
            }
        }

        // No numbers in this string, I guess? Can't do much with this.
        if(firstNumIndex == -1) { return std::string(); }

        // Trim trailing non-numeric values.
        // Usually just '}', but could be anything.
        std::string noBraces = string.substr(firstNumIndex);
        while(!noBraces.empty() && !std::isdigit(noBraces.back()))
        {
            noBraces.pop_back();
        }
        return noBraces;
    }

    inline void TrimToValidNumberString(std::string& numberString)
    {
        // The passed in string is expected to represent a single number. But there may be cruft on either end.
        // Guard against too many negative or decimals...perhaps due to typos.
        while(numberString.size() >= 2)
        {
            if((numberString[0] == '-' && numberString[1] == '-') ||
               (numberString[0] == '.' && numberString[1] == '.'))
            {
                numberString.erase(0, 1);
            }
            else
            {
                break;
            }
        }
    }
}