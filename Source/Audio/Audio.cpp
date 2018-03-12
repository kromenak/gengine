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
#include "mad.h"

Audio::Audio(std::string name, char* dataBuffer, int dataBufferLength) :
    Asset(name),
    mDataBuffer(dataBuffer),
    mDataBufferLength(dataBufferLength)
{
    ParseFromData(dataBuffer, dataBufferLength);
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

void Audio::ParseFromData(char* data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "RIFF".
    char identifier[4];
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "RIFF"))
    {
        std::cout << "WAV file does not have RIFF identifier!" << std::endl;
        return;
    }
    
    // Next is the size of the data.
    // This is always the entire file size, minus 8 bytes.
    unsigned int dataSize = reader.ReadUInt();
    
    // Next 4 bytes: WAVE file identifier "WAVE".
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "WAVE"))
    {
        std::cout << "WAV file does not have WAVE identifier!" << std::endl;
        return;
    }
    
    // Next, we have one or more chunks containing WAVE data.
    // The chunk identifier should be "fmt ".
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "fmt "))
    {
        std::cout << "WAV file chunk didn't start with 'fmt ' identifier!" << std::endl;
        return;
    }
    
    std::cout << "Current Position: " << reader.GetPosition() << std::endl;
    
    // 4 bytes: length of the format data.
    unsigned int formatChunkSize = reader.ReadUInt();
    
    // From here: "fact" chunk is formatChunkSize bytes ahead.
    // Or, for PCM, "data" chunk is formatChunkSize bytes ahead.
    // The next bit of data (format) indicates PCM vs other.
    
    // 2 bytes: the format used by the data.
    unsigned short format = reader.ReadUShort();
    if(format != 0x0055)
    {
        std::cout << "Unknown format tag " << format << std::endl;
        return;
    }
    else
    {
        std::cout << "Format is correct!" << std::endl;
    }
    
    // 2 bytes: the number of channels (1 = mono, 2 = stereo, etc).
    unsigned short numChannels = reader.ReadUShort();
    std::cout << "Num channels: " << numChannels << std::endl;
    
    // 4 bytes: the sample rate (e.g. 44100).
    unsigned int samplesPerSec = reader.ReadUInt();
    std::cout << "Sample rate: " << samplesPerSec << std::endl;
    
    // 4 bytes: data rate ((sampleRate * bitsPerSample * channels) / 8).
    unsigned int averageBytesPerSec = reader.ReadUInt();
    
    // 2 bytes: data block size in bytes (bitsPerSample * channels)
    unsigned short dataBlockSize = reader.ReadUShort();
    
    // 2 bytes: bits per sample
    unsigned short bitsPerSample = reader.ReadUShort();
    
    // 2 bytes: size of extension, which is appended to end of format chunk.
    unsigned short extensionSize = reader.ReadUShort();
    
    //TODO: Read extension data.
    reader.Skip(extensionSize);
    
    // If format is NOT PCM (0x0001), there is a "fact" chunk.
    if(format != 0x0001)
    {
        reader.Read(identifier, 4);
        if(!strcmp(identifier, "fact"))
        {
            std::cout << "Non-PCM WAV file is missing fact chunk!" << std::endl;
            return;
        }
        
        unsigned int factChunkSize = reader.ReadUInt();
        
        //TODO: Read fact chunk data.
        
        reader.Skip(factChunkSize);
    }
    
    // We should now be at the "data" chunk.
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "data"))
    {
        std::cout << "WAV file is missing data chunk!" << std::endl;
        return;
    }
    
    unsigned int dataChunkSize = reader.ReadUInt();
    std::cout << "Data chunk size is " << dataChunkSize << std::endl;
}
