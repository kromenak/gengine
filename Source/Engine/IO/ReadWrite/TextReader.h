//
// Clark Kromenaker
//
// Reads text from a stream.
//
#pragma once
#include "StreamReaderWriter.h"

#include <initializer_list>
#include <istream>

class TextReader : public StreamReader
{
public:
    TextReader(const char* filePath);
    TextReader(const char* memory, uint32_t memoryLength);
    TextReader(uint8_t* memory, uint32_t memoryLength);
    TextReader(std::istream* stream);

    std::string ReadLine();
    bool ReadLine(std::string& outString);
    uint64_t ReadLine(char* buffer, uint64_t bufferSize);

    //ReadString
    //Read numeric types

private:

};