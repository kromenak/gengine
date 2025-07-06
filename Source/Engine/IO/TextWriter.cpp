#include "TextWriter.h"

#include <iostream>

TextWriter::TextWriter(const char* filePath) :
    mStream(std::ofstream(filePath, std::ios::out))
{
    if(!mStream.good())
    {
        std::cout << "Can't write to text file " << filePath << std::endl;
    }
}
