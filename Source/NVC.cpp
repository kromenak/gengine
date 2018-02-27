//
// NVC.cpp
//
// Clark Kromenaker
//
#include "NVC.h"
#include "IniParser.h"

NVC::NVC(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void NVC::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Main section contains all the Noun/Verb/Cases on individual lines.
    IniSection mainSection = parser.GetSection("");
    for(auto& entry : mainSection.entries)
    {
        // The first three items must be the noun, verb, and case.
        std::string noun = entry->key;
    
        IniKeyValue* keyValue = entry->next;
        std::string verb = keyValue->key;
        
        keyValue = keyValue->next;
        std::string caseVal = keyValue->key;
        
        // From here, we have some optional stuff.
        keyValue = keyValue->next;
        while(keyValue != nullptr)
        {
            if(keyValue->key == "approach")
            {
                // Valid options are: WalkTo, Anim, Near, NearModel, Region, TurnTo, TurnToModel, WalkToSee
            }
            else if(keyValue->key == "target")
            {
                
            }
            else if(keyValue->key == "script")
            {
                // A sheep expression to be evaluated for this item.
            }
            keyValue = keyValue->next;
        }
    }
    
    // This section just contains one key/value pair per line.
    IniSection logicSection = parser.GetSection("LOGIC");
    for(auto& entry : logicSection.entries)
    {
        std::string caseLabel = entry->key;
        std::string sheepExpression = entry->value;
    }
}
