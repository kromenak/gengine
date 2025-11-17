//
// Clark Kromenaker
//
// Wrapper around an output stream that makes it easier to write out binary data.
//
#pragma once
#include "StreamReaderWriter.h"

#include <cstdint>
#include <ostream>

class BinaryWriter : public StreamWriter
{
public:
    BinaryWriter(const char* filePath);
    BinaryWriter(uint8_t* memory, uint32_t memoryLength);
    BinaryWriter(std::ostream* stream);

    // Write arbitrary data
    using StreamWriter::Write; // ensures Write function from base class is visible in this class
    void Write(const uint8_t* buffer, uint32_t size);

    // Write numeric types
    void WriteByte(uint8_t val);
    void WriteSByte(int8_t val);

    void WriteUShort(uint16_t val);
    void WriteShort(int16_t val);

    void WriteUInt(uint32_t val);
    void WriteInt(int32_t val);

    void WriteULong(uint64_t val);
    void WriteLong(int64_t val);

    void WriteFloat(float val);
    void WriteDouble(double val);

    // Write string w/o size info (reader better know size ahead of time).
    // If buffer size is specified, exactly that sized buffer will be written (with truncation or padding as needed).
    void WriteString(const std::string& str, uint32_t bufferSize = 0);

    // Write strings with size info encoded as 8/16/32-bit value.
    void WriteString8(const std::string& str);
    void WriteString16(const std::string& str);
    void WriteString32(const std::string& str);

private:

};