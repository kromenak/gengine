//
// Yak.cpp
//
// Clark Kromenaker
//
#include "Yak.h"

Yak::Yak(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void Yak::ParseFromData(char *data, int dataLength)
{
    
}
