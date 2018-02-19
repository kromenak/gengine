//
// SIF.cpp
//
// Clark Kromenaker
//
#include "SIF.h"
#include "IniParser.h"
#include <iostream>

SIF::SIF(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void SIF::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            std::string key = parser.GetKey();
            std::string value = parser.GetValue();
            if(value.empty())
            {
                std::cout << key << std::endl;
            }
            else
            {
                std::cout << key << " = " << value << std::endl;
            }
        }
    }
    std::cout << "Done." << std::endl;
}
