//
//  Audio.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/24/17.
//

#pragma once
#include <string>

using namespace std;

class Audio
{
public:
    
private:
    string mName = "";
    
    char* mDataBuffer = nullptr;
    int mDataBufferLength = 0;
    
public:
    Audio(string name, char* dataBuffer, int dataBufferLength);
    
    char* GetDataBuffer() const { return mDataBuffer; }
    int GetDataBufferLength() const { return mDataBufferLength; }
    
    void WriteToFile();
    
private:
    void ParseFromWaveFileData(char* dataBuffer, int dataBufferLength);
};
