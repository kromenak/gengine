//
// Clark Kromenaker
//
// Writes data to file in INI format.
//
#pragma once
#include "TextWriter.h"

class IniWriter
{
public:
    IniWriter(const char* filePath);

    void WriteSectionHeader(const char* sectionName);
    void WriteKeyValue(const char* key, const char* value);
    void WriteEmptyLine();

private:
    TextWriter mTextWriter;
};