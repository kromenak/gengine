//
//  BinaryReader.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/5/17.
//
#pragma once
#include <fstream>

class BinaryReader
{
public:
    BinaryReader(const char* filePath);
    BinaryReader(const char* memory, unsigned int memoryLength);
    ~BinaryReader();
    
	bool CanRead() const { return stream->good(); }
	bool IsEof() const { return stream->eof(); }
    
    void Seek(int position);
    void Skip(int size);
    
    int GetPosition();
    
    int Read(char* buffer, int size);
    int Read(unsigned char* buffer, int size);
    
    std::string ReadString(int length);
    
    uint8_t ReadUByte();
    int8_t ReadByte();
    
    uint16_t ReadUShort();
    int16_t ReadShort();
    
    uint32_t ReadUInt();
    int32_t ReadInt();
    
    float ReadFloat();
    double ReadDouble();
    
private:
    std::istream* stream = nullptr;
};
