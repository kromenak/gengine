//
//  BinaryReader.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/5/17.
//
#include "BinaryReader.h"

#include <iostream>
#include "imstream.h"

BinaryReader::BinaryReader(const char* filePath)
{
    stream = new std::ifstream(filePath, std::ios::in | std::ios::binary);
    if(!stream->good())
    {
        std::cout << "BinaryReader can't read from file " << filePath << "!" << std::endl;
    }
}

BinaryReader::BinaryReader(const char* memory, unsigned int memoryLength)
{
    stream = new imstream(memory, memoryLength);
}

BinaryReader::~BinaryReader()
{
    delete stream;
}

bool BinaryReader::CanRead() const
{
    return stream->good();
}

bool BinaryReader::IsEof() const
{
    return stream->eof();
}

void BinaryReader::Seek(int position)
{
    stream->seekg(position, std::ios::beg);
}

void BinaryReader::Skip(int size)
{
    stream->seekg(size, std::ios::cur);
}

int BinaryReader::GetPosition()
{
    return (int)stream->tellg();
}

void BinaryReader::Read(char* buffer, int size)
{
    stream->read(buffer, size);
}

void BinaryReader::Read(unsigned char* buffer, int size)
{
    stream->read((char*)buffer, size);
}

std::string BinaryReader::ReadString(int length)
{
    char* buffer = new char[length];
    stream->read(buffer, length);
    
    // Find null terminator, if any.
    for(int i = 0; i < length; i++)
    {
        if(buffer[i] == '\0')
        {
            return std::string(buffer, i);
        }
    }
    return std::string(buffer, length);
}

uint8_t BinaryReader::ReadUByte()
{
    uint8_t val;
    stream->read(reinterpret_cast<char*>(&val), 1);
    return val;
}

int8_t BinaryReader::ReadByte()
{
    int8_t val;
    stream->read(reinterpret_cast<char*>(&val), 1);
    return val;
}

uint16_t BinaryReader::ReadUShort()
{
    uint16_t val;
    stream->read(reinterpret_cast<char*>(&val), 2);
    return val;
}

int16_t BinaryReader::ReadShort()
{
    int16_t val;
    stream->read(reinterpret_cast<char*>(&val), 2);
    return val;
}

uint32_t BinaryReader::ReadUInt()
{
    uint32_t val;
    stream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

int32_t BinaryReader::ReadInt()
{
    int32_t val;
    stream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

float BinaryReader::ReadFloat()
{
    float val;
    stream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

double BinaryReader::ReadDouble()
{
    double val;
    stream->read(reinterpret_cast<char*>(&val), 8);
    return val;
}
