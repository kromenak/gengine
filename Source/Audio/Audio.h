//
//  Audio.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/24/17.
//
#pragma once
#include "Asset.h"
#include <string>

class Audio : Asset
{
public:
    Audio(std::string name, char* dataBuffer, int dataBufferLength);
    
    char* GetDataBuffer() const { return mDataBuffer; }
    int GetDataBufferLength() const { return mDataBufferLength; }
    
    void WriteToFile();
    
private:
    char* mDataBuffer = nullptr;
    int mDataBufferLength = 0;
    
    void ParseFromData(char* data, int dataLength);
};
