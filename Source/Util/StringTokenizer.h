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
    StringTokenizer(const std::string& str, std::initializer_list<char> splitChars);
    
    bool HasNext() const { return mIndex < mTokens.size(); }
    const std::string& GetNext();

    size_t GetTokenCount() const { return mTokens.size(); }

    void SetIndex(size_t index) { mIndex = index; }
    
private:
    std::vector<std::string> mTokens;
    size_t mIndex = 0;
};
