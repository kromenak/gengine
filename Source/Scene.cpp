//
// Scene.cpp
//
// Clark Kromenaker
//
#include "Scene.h"
#include "IniParser.h"
#include <iostream>

Scene::Scene(std::string name, char* data, int dataLength) : Asset(name)
{
    
}

void Scene::ParseFromData(char *data, int dataLength)
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
