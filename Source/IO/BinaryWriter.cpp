#include "BinaryWriter.h"

#include <fstream>

#include "mstream.h"

BinaryWriter::BinaryWriter(const char* filePath)
{
    mStream = new std::ofstream(filePath, std::ios::out | std::ios::binary);
}

BinaryWriter::BinaryWriter(char* memory, uint32_t memoryLength)
{
    mStream = new omstream(memory, memoryLength);
}

BinaryWriter::~BinaryWriter()
{
    delete mStream;
}

void BinaryWriter::Seek(int32_t position)
{
    // It's possible we've hit EOF, especially if we're jumping around a lot.
    // If we are trying to seek on an EOF stream, clear the error flags and do the seek.
    if(!mStream->good() && mStream->eof())
    {
        mStream->clear();
    }
    mStream->seekp(position, std::ios::beg);
}

void BinaryWriter::Skip(int32_t count)
{
    mStream->seekp(count, std::ios::cur);
}

void BinaryWriter::Write(const char* buffer, uint32_t size)
{
    mStream->write(buffer, size);
}

void BinaryWriter::Write(const unsigned char* buffer, uint32_t size)
{
    mStream->write(reinterpret_cast<const char*>(buffer), size);
}

void BinaryWriter::WriteByte(uint8_t val)
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

void BinaryWriter::WriteULong(uint64_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 8);
}

void BinaryWriter::WriteLong(int64_t val)
{
    mStream->write(reinterpret_cast<char*>(&val), 8);
}

void BinaryWriter::WriteFloat(float val)
{
    mStream->write(reinterpret_cast<char*>(&val), 4);
}

void BinaryWriter::WriteDouble(double val)
{
    mStream->write(reinterpret_cast<char*>(&val), 8);
}

void BinaryWriter::WriteString(const std::string& str)
{
    // Writing a string without recording its size!
    // This is fine if writer & reader have established a clear protocol.
    mStream->write(str.c_str(), str.size());
}

void BinaryWriter::WriteTinyString(const std::string& str)
{
    // A "tiny" string must be <= 255 chars.
    WriteByte(static_cast<uint8_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteShortString(const std::string& str)
{
    // A "short" string must be <= 65,535 chars (really quite a large string).
    WriteUShort(static_cast<uint16_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteMedString(const std::string& str)
{
    // A "medium" string can have length up to max size of unsigned 32-bit integer.
    WriteUInt(static_cast<uint32_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteLongString(const std::string& str)
{
    // A "long" string can have length up to max size of unsigned 64-bit integer.
    WriteULong(static_cast<uint64_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteStringBuffer(const std::string& str, uint32_t bufferSize)
{
    // Write the string data, but not more than the passed in size!
    // This *may* truncate the string data, if not careful.
    uint32_t writeSize = str.size();
    if(writeSize > bufferSize)
    {
        writeSize = bufferSize;
    }
    mStream->write(str.c_str(), writeSize);

    // Fill remaining space (if any) with null terminators.
    while(writeSize < bufferSize)
    {
        mStream->put('\0');
        ++writeSize;
    }
}