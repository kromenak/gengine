#include "BinaryReader.h"

#include <fstream>
#include <iostream>
#include "imstream.h"

BinaryReader::BinaryReader(const std::string& filePath) :
	BinaryReader(filePath.c_str())
{
	
}

BinaryReader::BinaryReader(const char* filePath)
{
    mStream = new std::ifstream(filePath, std::ios::in | std::ios::binary);
    if(!mStream->good())
    {
        std::cout << "BinaryReader can't read from file " << filePath << "!" << std::endl;
    }
}

BinaryReader::BinaryReader(const char* memory, unsigned int memoryLength)
{
    mStream = new imstream(memory, memoryLength);
}

BinaryReader::~BinaryReader()
{
    delete mStream;
}

void BinaryReader::Seek(int position)
{
	// It's possible we've hit EOF, especially if we're jumping around a lot.
	// If we are trying to seek on an EOF stream, clear the error flags and do the seek.
	if(!mStream->good() && mStream->eof())
	{
		mStream->clear();
	}
    mStream->seekg(position, std::ios::beg);
}

void BinaryReader::Skip(int size)
{
    mStream->seekg(size, std::ios::cur);
}

int BinaryReader::Read(char* buffer, int size)
{
    mStream->read(buffer, size);
	return (int)mStream->gcount();
}

int BinaryReader::Read(unsigned char* buffer, int size)
{
    mStream->read((char*)buffer, size);
	return (int)mStream->gcount();
}

std::string BinaryReader::ReadString(int length)
{
    char* buffer = new char[length];
    mStream->read(buffer, length);
	
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
    mStream->read(reinterpret_cast<char*>(&val), 1);
    return val;
}

int8_t BinaryReader::ReadByte()
{
    int8_t val;
    mStream->read(reinterpret_cast<char*>(&val), 1);
    return val;
}

uint16_t BinaryReader::ReadUShort()
{
    uint16_t val;
    mStream->read(reinterpret_cast<char*>(&val), 2);
    return val;
}

int16_t BinaryReader::ReadShort()
{
    int16_t val;
    mStream->read(reinterpret_cast<char*>(&val), 2);
    return val;
}

uint32_t BinaryReader::ReadUInt()
{
    uint32_t val;
    mStream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

int32_t BinaryReader::ReadInt()
{
    int32_t val;
    mStream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

float BinaryReader::ReadFloat()
{
    float val;
    mStream->read(reinterpret_cast<char*>(&val), 4);
    return val;
}

double BinaryReader::ReadDouble()
{
    double val;
    mStream->read(reinterpret_cast<char*>(&val), 8);
    return val;
}

Vector2 BinaryReader::ReadVector2()
{
    float x = ReadFloat();
    float y = ReadFloat();
    return Vector2(x, y);
}

Vector3 BinaryReader::ReadVector3()
{
    float x = ReadFloat();
    float y = ReadFloat();
    float z = ReadFloat();
    return Vector3(x, y, z);
}
