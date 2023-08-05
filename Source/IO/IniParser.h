//
// Clark Kromenaker
//
// Parses key/value data from INI-format files.
// Does contain some special-sauce unique to GK3.
//
#pragma once
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "Color32.h"
#include "Rect.h"
#include "Vector2.h"
#include "Vector3.h"

struct IniKeyValue
{
    std::string key;
    std::string value;
    
    float GetValueAsFloat() const;
    int GetValueAsInt() const;
    bool GetValueAsBool() const;
    Vector2 GetValueAsVector2() const;
    Vector3 GetValueAsVector3() const;
	Color32 GetValueAsColor32() const;
	Rect GetValueAsRect() const;
};

struct IniLine
{
	std::vector<IniKeyValue> entries;
};

struct IniSection
{
    std::string name;
    std::string condition;
	std::vector<IniLine> lines;
    
    std::unordered_map<std::string, IniKeyValue> GetAsMap() const;
};

class IniParser
{
public:
    IniParser(const char* filePath);
    IniParser(const uint8_t* memory, uint32_t memoryLength);
    ~IniParser();
    
    // MODE A: Read it all in at once and use it.
    void ParseAll();
    std::unordered_map<std::string, IniKeyValue> ParseAllAsMap();
    std::vector<IniSection> GetSections(const std::string& name);
    IniSection GetSection(const std::string& name);
    
    // MODE B: Read one section at a time.
    void ResetToTop();
    bool ReadNextSection(IniSection& sectionOut);
    
    // MODE C: Read line by line by line.
    bool ReadLine();
    bool ReadKeyValuePair();
    const std::string& GetLine() const { return mCurrentLine; }
    const IniKeyValue& GetKeyValue() { return mCurrentKeyValue; }
    
    void SetMultipleKeyValuePairsPerLine(bool areThere) { mMultipleKeyValuePairsPerLine = areThere; }
    
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
    
    // When we read in all at once or next section, we store all sections here.
    std::vector<IniSection> mSections;
    
    // If true, we assume that each line can have multiple key/value pairs.
    // As a result, we split based on commas. Otherwise, we assume whole line has one key/value pair.
    bool mMultipleKeyValuePairsPerLine = true;
};
