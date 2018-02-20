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

std::string Scene::GetBSPName()
{
    // If an override is specified, use it!
    if(!mBSPNameOverride.empty())
    {
        return mBSPNameOverride;
    }
    
    // Otherwise, we default to our asset name, with no extension.
    return mName.substr(0, mName.length() - 4);
}

void Scene::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    std::vector<IniSection> topSections = parser.GetSections("");
    for(auto& section : topSections)
    {
        for(auto& entry : section.entries)
        {
            if(entry->value == "BSP")
            {
                
            }
        }
    }
    
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            IniKeyValue keyValue = parser.GetKeyValue();
            if(keyValue.value.empty())
            {
                std::cout << keyValue.key << std::endl;
            }
            else
            {
                std::cout << keyValue.key << " = " << keyValue.value << std::endl;
            }
        }
    }
    std::cout << "Done." << std::endl;
}
