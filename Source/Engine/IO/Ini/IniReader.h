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
#include "Ini.h"
#include "Rect.h"
#include "TextReader.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class IniReader
{
public:
    IniReader(const char* filePath);
    IniReader(const uint8_t* memory, uint32_t memoryLength);
    IniReader(std::istream* stream);

    // Reader Configuration
    void SetMultipleKeyValuePairsPerLine(bool areThere) { mMultipleKeyValuePairsPerLine = areThere; }

    // MODE A: Read everything into memory and then query the data.
    void ReadAll();
    std::unordered_map<std::string, IniKeyValue> ReadAllAsMap();
    std::vector<IniSection> GetSections(const std::string& name);
    IniSection GetSection(const std::string& name);

    // MODE B: Read one section at a time.
    bool ReadNextSection(IniSection& sectionOut);

    // MODE C: Read line by line.
    bool ReadLine();
    bool ReadKeyValuePair();
    const IniKeyValue& GetKeyValue() { return mCurrentKeyValue; }

    const std::string& GetLine() const { return mCurrentLine; } //TODO: Get rid of this one!

private:
    // The stream object we use to read in the file contents.
    TextReader mTextReader;

    // When using "read everything" mode, holds all sections read into memory.
    std::vector<IniSection> mSections;

    // The section we are currently in. For example, if [General] in file, this might be "General".
    std::string mCurrentSectionName;
    std::string mCurrentSectionCondition;

    // Whatever the current line is. Can move to next line by calling "ReadLine".
    std::string mCurrentLine;

    // Values used when parsing a single line's key/value pairs.
    std::string mCurrentLineWorking;
    IniKeyValue mCurrentKeyValue;

    // If true, we assume that each line can have multiple key/value pairs.
    // As a result, we split based on commas. Otherwise, we assume whole line has one key/value pair.
    bool mMultipleKeyValuePairsPerLine = true;

    bool ReadNextKeyValueLine(bool stopAtSectionHeader);
    bool ParseKeyValuePairFromCurrentLine();
};
