#include "BinaryReader.h"

#include <fstream>

#include "mstream.h"

BinaryReader::BinaryReader(const char* filePath) :
    StreamReader(new std::ifstream(filePath, std::ios::in | std::ios::binary), true)
{
    //mStream = new std::ifstream(filePath, std::ios::in | std::ios::binary);
}

BinaryReader::BinaryReader(const uint8_t* memory, uint32_t memoryLength) :
    StreamReader(new imstream(reinterpret_cast<const char*>(memory), memoryLength), true)
{
    //mStream = new imstream(reinterpret_cast<const char*>(memory), memoryLength);
}

BinaryReader::BinaryReader(const char* memory, uint32_t memoryLength) :
    BinaryReader(reinterpret_cast<const uint8_t*>(memory), memoryLength)
{

}

BinaryReader::BinaryReader(std::istream* stream) : StreamReader(stream)
{

}

uint32_t BinaryReader::Read(uint8_t* buffer, uint32_t size)
{
    mStream->read(reinterpret_cast<char*>(buffer), size);
    return static_cast<uint32_t>(mStream->gcount());
}

uint8_t BinaryReader::ReadByte()
{
    uint8_t val;
    mStream->read(reinterpret_cast<char*>(&val), 1);
    return val;
}

int8_t BinaryReader::ReadSByte()
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

uint64_t BinaryReader::ReadULong()
{
    uint64_t val;
    mStream->read(reinterpret_cast<char*>(&val), 8);
    return val;
}

int64_t BinaryReader::ReadLong()
{
    int64_t val;
    mStream->read(reinterpret_cast<char*>(&val), 8);
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

std::string BinaryReader::ReadString(uint32_t length)
{
    std::string str;
    ReadString(length, str);
    return str;
}

void BinaryReader::ReadString(uint32_t size, std::string& str)
{
    // Make sure we've got enough capacity for the string data.
    // Use "resize" rather than "reserve" so that the string's size member is set correctly.
    // This implicitly ensures a null terminator is present as well.
    str.resize(size);

    // Directly modify the string data; a little dangerous!
    mStream->read(const_cast<char*>(str.data()), size);

    // Reduce string size if null terminator exists before end of string.
    for(size_t i = 0; i < str.size(); ++i)
    {
        if(str[i] == '\0')
        {
            str.resize(i);
        }
    }
}

std::string BinaryReader::ReadString8()
{
    uint8_t size = ReadByte();
    return ReadString(size);
}

void BinaryReader::ReadString8(std::string& str)
{
    uint8_t size = ReadByte();
    ReadString(static_cast<uint32_t>(size), str);
}

std::string BinaryReader::ReadString16()
{
    uint16_t size = ReadUShort();
    return ReadString(size);
}

void BinaryReader::ReadString16(std::string& str)
{
    uint16_t size = ReadUShort();
    ReadString(static_cast<uint32_t>(size), str);
}

std::string BinaryReader::ReadString32()
{
    uint32_t size = ReadUInt();
    return ReadString(size);
}

void BinaryReader::ReadString32(std::string& str)
{
    uint32_t size = ReadUInt();
    ReadString(size, str);
}

Vector2 BinaryReader::ReadVector2()
{
    Vector3 vector;
    mStream->read(reinterpret_cast<char*>(&vector), 8);
    return vector;
}

Vector3 BinaryReader::ReadVector3()
{
    Vector3 vector;
    mStream->read(reinterpret_cast<char*>(&vector), 12);
    return vector;
}
