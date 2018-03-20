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
#include "Vector2.h"
#include "Vector3.h"
#include <vector>
#include <unordered_map>

struct IniKeyValue
{
    std::string key;
    std::string value;
    
    IniKeyValue* next;
    
    float GetValueAsFloat() { return (float)atof(value.c_str()); }
    int GetValueAsInt() { return atoi(value.c_str()); }
    Vector2 GetValueAsVector2();
    Vector3 GetValueAsVector3();
};

struct IniSection
{
    std::string name;
    std::string condition;
    std::vector<IniKeyValue*> entries;
};

class IniParser
{
public:
    IniParser(const char* filePath);
    IniParser(const char* memory, unsigned int memoryLength);
    ~IniParser();
    
    // MODE A: Read it all in at once and use it.
    void ParseAll();
    std::vector<IniSection> GetSections(std::string name);
    IniSection GetSection(std::string name);
    
    // MODE B: Read one section at a time.
    void ResetToTop();
    bool ReadNextSection(IniSection& sectionOut);
    
    // MODE C: Read line by line by line.
    bool ReadLine();
    bool ReadKeyValuePair();
    IniKeyValue GetKeyValue() { return mCurrentKeyValue; }
    
private:
    // The stream object we use to read in the file contents.
    std::istream* mStream = nullptr;
    
    // The section we are currently in. For example, if [General] in file, this might be "General".
    std::string mCurrentSection;
    
    // Whatever the current line is. Can move to next line by calling "ReadLine".
    std::string mCurrentLine;
    
    // Values used when parsing a single line's key/value pairs.
    std::string mCurrentLineWorking;
    IniKeyValue mCurrentKeyValue;
    
    std::vector<IniSection> mSections;
};
