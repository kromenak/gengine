//
// StringTokenizer.cpp
//
// Clark Kromenaker
//
// Given a string, provides a way to get pieces of it one at a time.
//
#pragma once
#include <initializer_list>
#include <string>
#include <vector>

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
