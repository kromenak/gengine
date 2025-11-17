#include "BinaryWriter.h"

#include <fstream>

#include "mstream.h"

BinaryWriter::BinaryWriter(const char* filePath) :
    StreamWriter(new std::ofstream(filePath, std::ios::out | std::ios::binary), true)
{

}

BinaryWriter::BinaryWriter(uint8_t* memory, uint32_t memoryLength) :
    StreamWriter(new omstream(reinterpret_cast<char*>(memory), memoryLength), true)
{

}

BinaryWriter::BinaryWriter(std::ostream* stream) : StreamWriter(stream)
{

}

void BinaryWriter::Write(const uint8_t* buffer, uint32_t size)
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

void BinaryWriter::WriteString(const std::string& str, uint32_t bufferSize)
{
    // Determine size of string.
    // If a buffer size is specified, the size can't exceed that length.
    uint32_t writeSize = str.size();
    if(bufferSize > 0 && writeSize > bufferSize)
    {
        writeSize = bufferSize;
    }

    // Writing a string without recording its size!
    // This is fine if writer & reader have established a clear protocol.
    mStream->write(str.c_str(), writeSize);

    // If a buffer size was specified, make sure we write the full buffer.
    // Fill remaining space (if any) with zero (null terminators).
    if(bufferSize > 0)
    {
        while(writeSize < bufferSize)
        {
            mStream->put('\0');
            ++writeSize;
        }
    }
}

void BinaryWriter::WriteString8(const std::string& str)
{
    // String must be <= 255 chars.
    WriteByte(static_cast<uint8_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteString16(const std::string& str)
{
    // String must be <= 65,535 chars (really quite a large string).
    WriteUShort(static_cast<uint16_t>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteString32(const std::string& str)
{
    // String can have length up to max size of unsigned 32-bit integer.
    WriteUInt(static_cast<uint32_t>(str.size()));
    WriteString(str);
}