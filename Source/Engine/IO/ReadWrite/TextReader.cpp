#include "TextReader.h"

#include <fstream>
#include <string>

#include "mstream.h"
#include "StringUtil.h"

TextReader::TextReader(const char* filePath) :
    StreamReader(new std::ifstream(filePath, std::ios::in | std::ios::binary), true)
{

}

TextReader::TextReader(const char* memory, uint32_t memoryLength) :
    StreamReader(new imstream(memory, memoryLength), true)
{

}

TextReader::TextReader(uint8_t* memory, uint32_t memoryLength) :
    TextReader(reinterpret_cast<char*>(memory), memoryLength)
{

}

TextReader::TextReader(std::istream* stream) : StreamReader(stream)
{

}

std::string TextReader::ReadLine()
{
    std::string toReturn;
    std::getline(*mStream, toReturn);
    StringUtil::TrimSpecialChars(toReturn);
    return toReturn;
}

bool TextReader::ReadLine(std::string& outString)
{
    std::getline(*mStream, outString);
    StringUtil::TrimSpecialChars(outString);
    return mStream->operator bool(); // use istream operator bool overload; returns true if last read succeeded
}

uint64_t TextReader::ReadLine(char* buffer, uint64_t bufferSize)
{
    mStream->getline(buffer, bufferSize);
    //TODO: Get rid of \r in this buffer, if any?
    return mStream->gcount();
}
