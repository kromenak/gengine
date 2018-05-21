//
// BinaryWriter.h
//
// Clark Kromenaker
//
// Wrapper around an output stream that makes it easier
// to write out binary data.
//
#pragma once
#include <fstream>

class BinaryWriter
{
public:
    BinaryWriter(const char* filePath);
    ~BinaryWriter();
    
    bool CanWrite() const;
    bool IsEof() const;
    
    void Seek(int position);
    void Skip(int size);
    
    int GetPosition();
    
    void Write(char* buffer, int size);
    void Write(unsigned char* buffer, int size);
    
    void WriteString(std::string val);
    
    void WriteUByte(uint8_t val);
    void WriteSByte(int8_t val);
    
    void WriteUShort(uint16_t val);
    void WriteShort(int16_t val);
    
    void WriteUInt(uint32_t val);
    void WriteInt(int32_t val);
    
    void WriteFloat(float val);
    void WriteDouble(double val);
    
private:
    std::ostream* stream = nullptr;
};
