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
    ~BinaryReader();
    
    void Seek(int position);
    void Skip(int size);
    
    void Read(char* charBuffer, int size);
    
    uint8_t ReadUByte();
    int8_t ReadByte();
    
    uint16_t ReadUShort();
    int16_t ReadShort();
    
    uint32_t ReadUInt();
    int32_t ReadInt();
    
    
private:
    std::ifstream fileStream;
};
