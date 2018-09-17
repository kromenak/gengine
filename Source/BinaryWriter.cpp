//
// BinaryWriter.cpp
//
// Clark Kromenaker
//
#include "BinaryWriter.h"

#include <iostream>

BinaryWriter::BinaryWriter(const char* filePath)
{
    stream = new std::ofstream(filePath, std::ios::out | std::ios::binary);
    if(!stream->good())
    {
        std::cout << "BinaryWriter can't write to file " << filePath << "!" << std::endl;
    }
}

BinaryWriter::~BinaryWriter()
{
    
    delete stream;
}

bool BinaryWriter::CanWrite() const
{
    return stream->good();
}

bool BinaryWriter::IsEof() const
{
    return stream->eof();
}

void BinaryWriter::Seek(int position)
{
    stream->seekp(position, std::ios::beg);
}

void BinaryWriter::Skip(int size)
{
    stream->seekp(size, std::ios::cur);
}

int BinaryWriter::GetPosition()
{
    return (int)stream->tellp();
}

void BinaryWriter::Write(char* buffer, int size)
{
    stream->write(buffer, size);
}

void BinaryWriter::Write(unsigned char* buffer, int size)
{
    stream->write((char*)buffer, size);
}

void BinaryWriter::WriteString(std::string val)
{
    stream->write(val.c_str(), val.size());
}

void BinaryWriter::WriteUByte(uint8_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 1);
}

void BinaryWriter::WriteSByte(int8_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 1);
}

void BinaryWriter::WriteUShort(uint16_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 2);
}

void BinaryWriter::WriteShort(int16_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 2);
}

void BinaryWriter::WriteUInt(uint32_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteInt(int32_t val)
{
    stream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteFloat(float val)
{
    stream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteDouble(double val)
{
    stream->write(reinterpret_cast<char*>(&val), 8);
}
