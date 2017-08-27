//
//  Audio.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/24/17.
//

#include "Audio.h"
#include <iostream>
#include <fstream>
#include "BinaryReader.h"

Audio::Audio(string name, char* dataBuffer, int dataBufferLength) :
    mName(name),
    mDataBuffer(dataBuffer),
    mDataBufferLength(dataBufferLength)
{
    ParseFromWaveFileData(dataBuffer, dataBufferLength);
}

void Audio::WriteToFile()
{
    std::ofstream fileStream(mName);
    if(fileStream.good())
    {
        fileStream.write(mDataBuffer, mDataBufferLength);
        fileStream.close();
        std::cout << "Wrote out " << mName << std::endl;
    }
}

void Audio::ParseFromWaveFileData(char *dataBuffer, int dataBufferLength)
{
    BinaryReader reader(dataBuffer, dataBufferLength);
    
    char fileType[5];
    reader.Read(fileType, 4);
    fileType[4] = '\0';
    
    std::cout << "File type identifier: " << fileType << endl;
}
