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
    Audio(const std::string& name, AssetScope scope) : Asset(name, scope) { }
	~Audio();

    void Load(uint8_t* data, uint32_t dataLength);
    
    uint8_t* GetDataBuffer() const { return mDataBuffer; }
    uint32_t GetDataBufferLength() const { return mDataBufferLength; }
    
    float GetDuration() const { return mDuration; }
    
private:
	const unsigned short kFormatPCM = 0x0001;
	//const unsigned short kMp3Format = 0x0055;
	
    // Audio data buffer - the contents of WAV file in memory.
    uint8_t* mDataBuffer = nullptr;
    uint32_t mDataBufferLength = 0;
    
    // The length of the audio file, calculated from taking (data size / samples per second).
    float mDuration = 0.0f;
};
