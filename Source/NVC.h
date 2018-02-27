//
// NVC.h
//
// Clark Kromenaker
//
// A (noun, verb, case) file, also called an "action" file.
// Specifies actions available to player in a specific location
// during a specific time or range of times.
//
// In-memory representation of .NVC assets.
//
#pragma once
#include "Asset.h"

class NVC : public Asset
{
public:
    NVC(std::string name, char* data, int dataLength);
    
private:
    void ParseFromData(char* data, int dataLength);
};
