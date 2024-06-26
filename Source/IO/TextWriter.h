//
// Clark Kromenaker
//
// Writes text to a stream.
// Currently only does files, but could write to other streams I suppose, with a little work...
//
#pragma once
#include <fstream>
#include <initializer_list>

class TextWriter
{
public:
    TextWriter(const char* filePath);

    void Write(const char* text) { mStream << text; }
    void EndLine() { mStream << "\n"; }

    void WriteLine(const char* line) { mStream << line << "\n"; }
    void WriteLine(const std::string& line) { mStream << line << "\n"; }

    void Write(std::initializer_list<const char*> list)
    {
        for(const char* item : list)
        {
            mStream << item;
        }
    }
    void WriteLine(std::initializer_list<const char*> list)
    {
        for(const char* item : list)
        {
            mStream << item;
        }
        mStream << "\n";
    }

private:
    std::ofstream mStream;
};