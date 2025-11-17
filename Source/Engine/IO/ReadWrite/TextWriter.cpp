#include "TextWriter.h"

#include <fstream>
#include <iostream>

TextWriter::TextWriter(const char* filePath) :
    StreamWriter(new std::ofstream(filePath, std::ios::out | std::ios::binary), true)
{

}

TextWriter::TextWriter(std::ostream* stream) : StreamWriter(stream)
{

}

void TextWriter::Write(std::initializer_list<const char*> list)
{
    for(const char* item : list)
    {
        *mStream << item;
    }
}

void TextWriter::Write(std::initializer_list<std::string> list)
{
    for(const std::string& item : list)
    {
        *mStream << item;
    }
}

void TextWriter::WriteLine(std::initializer_list<const char*> list)
{
    for(const char* item : list)
    {
        *mStream << item;
    }
    *mStream << "\n";
}

void TextWriter::WriteLine(std::initializer_list<std::string> list)
{
    for(const std::string& item : list)
    {
        *mStream << item;
    }
    *mStream << "\n";
}