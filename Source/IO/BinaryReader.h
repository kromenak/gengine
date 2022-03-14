//
// Clark Kromenaker
//
// Wrapper around a binary data stream, with helpers for reading bytes as specific types.
//
#pragma once
#include <cstdint>
#include <istream>

#include "Vector2.h"
#include "Vector3.h"

class BinaryReader
{
public:
    BinaryReader(const char* filePath);
    BinaryReader(const char* memory, uint32_t memoryLength);
    ~BinaryReader();
	
    // Should only write if OK is true.
    // Remember, "good" returns true as long as fail/bad/eof bits are all false.
    bool OK() const { return mStream->good(); }

    // Position
    void Seek(int32_t position);
    void Skip(int32_t count);
    int32_t GetPosition() const { return static_cast<int32_t>(mStream->tellg()); }

    // Read arbitrary char data
    int32_t Read(char* buffer, uint32_t size);
    int32_t Read(unsigned char* buffer, uint32_t size);

    // Read numeric types
    uint8_t ReadByte();
    int8_t ReadSByte();
    
    uint16_t ReadUShort();
    int16_t ReadShort();
    
    uint32_t ReadUInt();
    int32_t ReadInt();

    uint64_t ReadULong();
    int64_t ReadLong();
    
    float ReadFloat();
    double ReadDouble();

    // Read strings of known size
    std::string ReadString(uint32_t size);
    void ReadString(uint32_t size, std::string& str);
    void ReadString(uint64_t size, std::string& str);

    // Read strings w/ size encoded as 8/16/32/64-bit value
    std::string ReadTinyString();
    void ReadTinyString(std::string& str);

    std::string ReadShortString();
    void ReadShortString(std::string& str);

    std::string ReadMedString();
    void ReadMedString(std::string& str);

    std::string ReadLongString();
    void ReadLongString(std::string& str);

    // Read string from fixed-size buffer.
    std::string ReadStringBuffer(uint32_t bufferSize);
    void ReadStringBuffer(uint32_t bufferSize, std::string& str);

    // For convenience - reading in some more commonly encountered complex types.
    Vector2 ReadVector2();
    Vector3 ReadVector3();

private:
	// Stream we are reading from.
	// Needs to be pointer because type of stream (memory, file, etc) changes sometimes.
    std::istream* mStream = nullptr;
};
