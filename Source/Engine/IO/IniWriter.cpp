#include "IniWriter.h"

IniWriter::IniWriter(const char* filePath) :
    mTextWriter(filePath)
{

}

void IniWriter::WriteSectionHeader(const char* sectionName)
{
    mTextWriter.WriteLine({"[", sectionName, "]"});
}

void IniWriter::WriteKeyValue(const char* key, const char* value)
{
    mTextWriter.WriteLine({key, " = ", value});
}

void IniWriter::WriteEmptyLine()
{
    mTextWriter.EndLine();
}
