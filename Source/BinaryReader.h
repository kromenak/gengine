//
//  BinaryReader.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/5/17.
//
#pragma once
#include <istream>

class BinaryReader
{
public:
	BinaryReader(const std::string& filePath);
    BinaryReader(const char* filePath);
    BinaryReader(const char* memory, unsigned int memoryLength);
    ~BinaryReader();
    
	bool CanRead() const { return mStream->good(); }
	bool IsEof() const { return mStream->eof(); }
    
    void Seek(int position);
    void Skip(int size);
    
	int GetPosition() const { return (int)mStream->tellg(); }
    
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
	// Stream we are reading from.
	// Needs to be pointer because type of stream (memory, file, etc) changes sometimes.
    std::istream* mStream = nullptr;
};
