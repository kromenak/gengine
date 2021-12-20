#include "BinaryWriter.h"

#include <iostream>

BinaryWriter::BinaryWriter(const char* filePath)
{
    mStream = new std::ofstream(filePath, std::ios::out | std::ios::binary);
    if(!mStream->good())
    {
        std::cout << "BinaryWriter can't write to file " << filePath << "!" << std::endl;
    }
}

BinaryWriter::~BinaryWriter()
{
    delete mStream;
}

void BinaryWriter::Seek(int position)
{
    mStream->seekp(position, std::ios::beg);
}

void BinaryWriter::Skip(int size)
{
    mStream->seekp(size, std::ios::cur);
}

void BinaryWriter::Write(char* buffer, int size)
{
    mStream->write(buffer, size);
}

void BinaryWriter::Write(unsigned char* buffer, int size)
{
    mStream->write((char*)buffer, size);
}

void BinaryWriter::WriteString(std::string val)
{
    mStream->write(val.c_str(), val.size());
}

void BinaryWriter::WriteUByte(uint8_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 1);
}

void BinaryWriter::WriteSByte(int8_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 1);
}

void BinaryWriter::WriteUShort(uint16_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 2);
}

void BinaryWriter::WriteShort(int16_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 2);
}

void BinaryWriter::WriteUInt(uint32_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteInt(int32_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteFloat(float val)
{
    mStream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteDouble(double val)
{
    mStream->write(reinterpret_cast<char*>(&val), 8);
}
