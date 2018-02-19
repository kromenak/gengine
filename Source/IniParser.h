//
// IniParser.h
//
// Clark Kromenaker
//
// A text parser that makes it easier to read INI-style
// text assets used in GK3.
//
#pragma once
#include <fstream>

class IniParser
{
public:
    IniParser(const char* filePath);
    IniParser(const char* memory, unsigned int memoryLength);
    ~IniParser();
    
    bool ReadLine();
    std::string GetSection() { return mCurrentSection; }
    std::string GetLine() { return mCurrentLine; }
    
    bool ReadKeyValuePair();
    std::string GetKey() { return mCurrentKey; }
    std::string GetValue() { return mCurrentValue; }
    
private:
    // The stream object we use to read in the file contents.
    std::istream* mStream = nullptr;
    
    // The section we are currently in. For example, if [General] in file, this might be "General".
    std::string mCurrentSection;
    
    // Whatever the current line is. Can move to next line by calling "ReadLine".
    std::string mCurrentLine;
    
    // Values used when parsing a single line's key/value pairs.
    std::string mCurrentLineWorking;
    std::string mCurrentKey;
    std::string mCurrentValue;
};
