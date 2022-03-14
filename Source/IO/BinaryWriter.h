//
// Clark Kromenaker
//
// Wrapper around an output stream that makes it easier to write out binary data.
//
#pragma once
#include <cstdint>
#include <ostream>

class BinaryWriter
{
public:
    BinaryWriter(const char* filePath);
    BinaryWriter(char* memory, uint32_t memoryLength);
    ~BinaryWriter();
	
	// Should only write if OK is true.
    // Remember, "good" returns true as long as fail/bad/eof bits are all false.
    bool OK() const { return mStream->good(); }

    // Position
    void Seek(int32_t position);
    void Skip(int32_t count);
    int32_t GetPosition() const { return static_cast<int32_t>(mStream->tellp()); }

    // Write arbitrary char data
    void Write(const char* buffer, uint32_t size);
    void Write(const unsigned char* buffer, uint32_t size);

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

    // Write string w/o size info (reader better know size ahead of time)
    void WriteString(const std::string& str);

    // Write strings with size info encoded as 8/16/32/64-bit value
    void WriteTinyString(const std::string& str);
    void WriteShortString(const std::string& str);
    void WriteMedString(const std::string& str);
    void WriteLongString(const std::string& str);

    // Writes string to fixed-size buffer w/ null terminator
    void WriteStringBuffer(const std::string& str, uint32_t bufferSize);

private:
    // Stream we are writing to.
    // Needs to be pointer because type of stream (memory, file, etc) changes sometimes.
    std::ostream* mStream = nullptr;
};
