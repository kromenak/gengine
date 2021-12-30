//
// Clark Kromenaker
//
// Writes text to a stream.
// Currently only does files, but could write to other streams I suppose, with a little work...
//
#pragma once
#include <fstream>

class TextWriter
{
public:
    TextWriter(const char* filePath);

    void WriteLine(const char* line);
    void WriteLine(const std::string& line);

private:
    std::ofstream mStream;
};