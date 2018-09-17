//
// NVC.cpp
//
// Clark Kromenaker
//
#include "NVC.h"

#include "IniParser.h"
#include "SheepCompiler.h"
#include "SheepScript.h"

NVC::NVC(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

NVCItem* NVC::GetNVC(std::string noun, std::string verb)
{
    // See if we have an entry for this noun. If not, we're done.
    auto it = mNounToItems.find(noun);
    if(it == mNounToItems.end()) { return nullptr; }
    
    // See if we have an item with the given verb.
    std::vector<NVCItem> items = mNounToItems[noun];
    for(auto& item : items)
    {
        if(item.verb == verb)
        {
            return &item;
        }
    }
    
    // Found no matching verb entry.
    return nullptr;
}

void NVC::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Main section contains all the Noun/Verb/Cases on individual lines.
    IniSection mainSection = parser.GetSection("");
    for(auto& entry : mainSection.entries)
    {
        NVCItem item;
        
        // The first three items must be the noun, verb, and case.
        item.noun = entry->key;
        
        IniKeyValue* keyValue = entry->next;
        item.verb = keyValue->key;
        
        keyValue = keyValue->next;
        item.condition = keyValue->key;
        
        // From here, we have some optional stuff.
        keyValue = keyValue->next;
        while(keyValue != nullptr)
        {
            if(keyValue->key == "approach")
            {
                // Valid options are: WalkTo, Anim, Near, NearModel, Region, TurnTo, TurnToModel, WalkToSee
                item.approach = keyValue->value;
            }
            else if(keyValue->key == "target")
            {
                item.target = keyValue->value;
            }
            else if(keyValue->key == "script")
            {
                // A sheep expression to be evaluated for this item.
                SheepCompiler compiler;
                item.script = compiler.Compile(keyValue->value);
            }
            keyValue = keyValue->next;
        }
        
        // Add item to map.
        auto it = mNounToItems.find(item.noun);
        if(it == mNounToItems.end())
        {
            mNounToItems[item.noun] = std::vector<NVCItem>();
        }
        mNounToItems[item.noun].push_back(item);
    }
    
    // Some "CASE" values are special, and handled by the system (like ALL, GABE_ALL, GRACE_ALL)
    // But an NVC item can also specify a custom case value. In that case,
    // this section maps the case value to a sheep expression to evaluate, to see whether the case is met.
    IniSection logicSection = parser.GetSection("LOGIC");
    for(auto& entry : logicSection.entries)
    {
        // Only add this case entry if it isn't a duplicate entry.
        std::string caseLabel = entry->key;
        auto it = mCaseToSheep.find(caseLabel);
        if(it == mCaseToSheep.end())
        {
            SheepCompiler compiler;
            mCaseToSheep[caseLabel] = compiler.Compile(entry->value);
        }
        else
        {
            std::cout << "Multiple case labels for " << caseLabel << std::endl;
        }
    }
}
