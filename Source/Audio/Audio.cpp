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

//const ushort kMp3Format = 0x0055;

Audio::Audio(std::string name, char* data, int dataLength) :
    Asset(name),
    mDataBuffer(data),
    mDataBufferLength(dataLength)
{
    ParseFromData(data, dataLength);
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
    
    // First 4 bytes: chunk ID "RIFF".
    std::string identifier = reader.ReadString(4);
    if(identifier != "RIFF")
    {
        std::cout << "WAV file does not have RIFF identifier!" << std::endl;
        return;
    }
    
    // Next is the chunk size, minus 8 bytes (for what we've already read).
    reader.ReadUInt();
    
    // Next 4 bytes: format ID "WAVE".
    identifier = reader.ReadString(4);
    if(identifier != "WAVE")
    {
        std::cout << "WAV file does not have WAVE identifier!" << std::endl;
        return;
    }
    
    // Next, we have one or more chunks containing WAVE data.
    // The subchunk identifier should be "fmt ".
    identifier = reader.ReadString(4);
    if(identifier != "fmt ")
    {
        std::cout << "WAV file chunk didn't start with 'fmt ' identifier!" << std::endl;
        return;
    }
    
    // 4 bytes: length of the subchunk data.
    reader.ReadUInt();
    
    // From here: "fact" chunk is formatChunkSize bytes ahead.
    // Or, for PCM, "data" chunk is formatChunkSize bytes ahead.
    // The next bit of data (format) indicates PCM vs other.
    
    // 2 bytes: the format used by the data.
    ushort format = reader.ReadUShort();
    //mIsMusic = (format == kMp3Format);
    
    // 2 bytes: the number of channels (1 = mono, 2 = stereo, etc).
    reader.ReadUShort();
    
    // 4 bytes: the sample rate (e.g. 44100).
    reader.ReadUInt();
    
    // 4 bytes: data rate ((sampleRate * bitsPerSample * channels) / 8).
    unsigned int byteRate = reader.ReadUInt();
    
    // 2 bytes: data block size in bytes (bitsPerSample * channels)
    // 2 bytes: bits per sample
    reader.Skip(4);
    
    // 2 bytes: size of extension, which is appended to end of format chunk.
    //TODO: Read extension data.
    unsigned short extensionSize = reader.ReadUShort();
    reader.Skip(extensionSize);
    
    // If format is NOT PCM (0x0001), there is a "fact" chunk.
    if(format != 0x0001)
    {
        identifier = reader.ReadString(4);
        if(identifier != "fact")
        {
            std::cout << "Non-PCM WAV file is missing fact chunk!" << std::endl;
            return;
        }
        
        //TODO: Read fact chunk data.
        unsigned int factChunkSize = reader.ReadUInt();
        reader.Skip(factChunkSize);
    }
    
    // We should now be at the "data" chunk.
    identifier = reader.ReadString(4);
    if(identifier != "data")
    {
        std::cout << "WAV file is missing data chunk!" << std::endl;
        return;
    }
    
    unsigned int dataChunkSize = reader.ReadUInt();
    //std::cout << "Data chunk size is " << dataChunkSize << std::endl;
    
    mDuration = dataChunkSize / byteRate;
}
