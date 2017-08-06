//
//  BinaryReader.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/5/17.
//

#include "BinaryReader.h"
#include <iostream>

BinaryReader::BinaryReader(const char* filePath) :
    fileStream(filePath, std::ios::in | std::ios::binary)
{
    if(!fileStream.good())
    {
        std::cout << "File's not good!" << std::endl;
    }
}

BinaryReader::~BinaryReader()
{
    fileStream.close();
}

void BinaryReader::Seek(int position)
{
    fileStream.seekg(position);
}

void BinaryReader::Skip(int size)
{
    fileStream.seekg(size, std::ios::seekdir::cur);
}

void BinaryReader::Read(char* charBuffer, int size)
{
    fileStream.read(charBuffer, size);
}

uint8_t BinaryReader::ReadUByte()
{
    uint8_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 1);
    return val;
}

int8_t BinaryReader::ReadByte()
{
    int8_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 1);
    return val;
}

uint16_t BinaryReader::ReadUShort()
{
    uint16_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 2);
    return val;
}

int16_t BinaryReader::ReadShort()
{
    int16_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 2);
    return val;
}

uint32_t BinaryReader::ReadUInt()
{
    uint32_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 4);
    return val;
}

int32_t BinaryReader::ReadInt()
{
    int32_t val;
    fileStream.read(reinterpret_cast<char*>(&val), 4);
    return val;
}
