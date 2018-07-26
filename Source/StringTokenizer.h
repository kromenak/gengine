//
// StringTokenizer.cpp
//
// Clark Kromenaker
//
// Given a string, provides a way to pieces of it one at a time.
//
#pragma once
#include <string>
#include <vector>
#include <initializer_list>

class StringTokenizer
{
public:
    StringTokenizer(std::string str, std::initializer_list<char> splitChars);
    
    bool HasNext() { return mIndex < mTokens.size(); }
    std::string GetNext();
    
private:
    std::vector<std::string> mTokens;
    int mIndex = 0;
};
