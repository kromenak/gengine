//
// Yak.h
//
// Clark Kromenaker
//
// Description goes here!
//
#pragma once
#include "Asset.h"

class Yak : public Asset
{
public:
    Yak(std::string name, char* data, int dataLength);
    
private:
    void ParseFromData(char* data, int dataLength);
};
