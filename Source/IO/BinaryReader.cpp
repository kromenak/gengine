#include "BinaryReader.h"

#include <fstream>

#include "mstream.h"

BinaryReader::BinaryReader(const char* filePath)
{
    mStream = new std::ifstream(filePath, std::ios::in | std::ios::binary);
}

BinaryReader::BinaryReader(const char* memory, uint32_t memoryLength)
{
    mStream = new imstream(memory, memoryLength);
}

BinaryReader::~BinaryReader()
{
    delete mStream;
}

void BinaryReader::Seek(int32_t position)
{
	// It's possible we've hit EOF, especially if we're jumping around a lot.
	// If we are trying to seek on an EOF stream, clear the error flags and do the seek.
	if(!mStream->good() && mStream->eof())
	{
		mStream->clear();
	}
    mStream->seekg(position, std::ios::beg);
}

void BinaryReader::Skip(int32_t count)
{
    mStream->seekg(count, std::ios::cur);
}

int32_t BinaryReader::Read(char* buffer, uint32_t size)
{
    mStream->read(buffer, size);
    return (int)mStream->gcount();
}

int32_t BinaryReader::Read(unsigned char* buffer, uint32_t size)
{
    mStream->read((char*)buffer, size);
    return (int)mStream->gcount();
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
}

void BinaryReader::ReadString(uint64_t size, std::string& str)
{
    // Same as above, but for insanely long string!? Would this ever be used???
    // Also, this probably won't work in 32-bit apps anyway, since std::string size_t would be 32-bit.
    str.resize(size);
    mStream->read(const_cast<char*>(str.data()), size);
}

std::string BinaryReader::ReadTinyString()
{
    uint8_t size = ReadByte();
    return ReadString(size);
}

void BinaryReader::ReadTinyString(std::string& str)
{
    uint8_t size = ReadByte();
    ReadString(static_cast<uint32_t>(size), str);
}

std::string BinaryReader::ReadShortString()
{
    uint16_t size = ReadUShort();
    return ReadString(size);
}

void BinaryReader::ReadShortString(std::string& str)
{
    uint16_t size = ReadUShort();
    ReadString(static_cast<uint32_t>(size), str);
}

std::string BinaryReader::ReadMedString()
{
    uint32_t size = ReadUInt();
    return ReadString(size);
}

void BinaryReader::ReadMedString(std::string& str)
{
    uint32_t size = ReadUInt();
    ReadString(size, str);
}

std::string BinaryReader::ReadLongString()
{
    uint64_t size = ReadULong();
    std::string str;
    ReadString(size, str);
    return str;
}

void BinaryReader::ReadLongString(std::string& str)
{
    uint64_t size = ReadULong();
    ReadString(size, str);
}

std::string BinaryReader::ReadStringBuffer(uint32_t bufferSize)
{
    std::string str;
    ReadStringBuffer(bufferSize, str);
    return str;
}

void BinaryReader::ReadStringBuffer(uint32_t bufferSize, std::string& str)
{
    // Read string per usual. BUT this may give incorrect results:
    // If buffer size is greater than string size (due to null terminator), str.size() is incorrect
    ReadString(bufferSize, str);

    // Reduce string size if null terminator exists before end of string.
    for(int i = 0; i < str.size(); ++i)
    {
        if(str[i] == '\0')
        {
            str.resize(i);
        }
    }
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
