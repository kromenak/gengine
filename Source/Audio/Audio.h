//
// Clark Kromenaker
//
// Audio data - feed to audio system to hear it!
//
#pragma once
#include "Asset.h"

#include <string>

class Audio : public Asset
{
public:
    Audio(const std::string& name, char* data, int dataLength);
	~Audio();
    
    char* GetDataBuffer() const { return mDataBuffer; }
    int GetDataBufferLength() const { return mDataBufferLength; }
    
    float GetDuration() const { return mDuration; }
    
private:
	const unsigned short kFormatPCM = 0x0001;
	//const unsigned short kMp3Format = 0x0055;
	
    // Audio data buffer - the contents of WAV file in memory.
    char* mDataBuffer = nullptr;
    int mDataBufferLength = 0;
    
    // What type of audio is this?
    bool mIsMusic = false;
    
    // The length of the audio file, calculated from taking (data size / samples per second).
    float mDuration = 0.0f;
    
    void ParseFromData(char* data, int dataLength);
};
