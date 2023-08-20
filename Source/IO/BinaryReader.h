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
    BinaryReader(const uint8_t* memory, uint32_t memoryLength);
    BinaryReader(const char* memory, uint32_t memoryLength);
    ~BinaryReader();
	
    // Should only write if OK is true.
    // Remember, "good" returns true as long as fail/bad/eof bits are all false.
    bool OK() const { return mStream->good(); }

    // Position
    void Seek(uint32_t position);
    void Skip(uint32_t count);
    uint32_t GetPosition() const;

    // Read arbitrary char data
    uint32_t Read(uint8_t* buffer, uint32_t size);
    uint32_t Read(char* buffer, uint32_t size);

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

    // Read strings of fixed maximum size.
    std::string ReadString(uint32_t size);
    void ReadString(uint32_t size, std::string& str);

    // Read strings w/ size encoded as 8/16/32-bit values
    std::string ReadString8();
    void ReadString8(std::string& str);

    std::string ReadString16();
    void ReadString16(std::string& str);

    std::string ReadString32();
    void ReadString32(std::string& str);

    // For convenience - reading in some more commonly encountered complex types.
    Vector2 ReadVector2();
    Vector3 ReadVector3();

private:
	// Stream we are reading from.
	// Needs to be pointer because type of stream (memory, file, etc) changes sometimes.
    std::istream* mStream = nullptr;
};
