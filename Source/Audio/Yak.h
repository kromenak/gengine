//
// Yak.h
//
// Clark Kromenaker
//
// Description goes here!
//
#pragma once
#include "Asset.h"

#include <vector>

class Yak : public Asset
{
public:
    Yak(std::string name, char* data, int dataLength);
    
    void Play(int numLines);
    
private:
    std::vector<std::string> mSounds;
    
    void ParseFromData(char* data, int dataLength);
};
