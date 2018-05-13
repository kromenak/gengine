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
#include <unordered_map>
#include <vector>

class SheepScript;

struct NVCItem
{
    std::string noun;
    std::string verb;
    std::string condition;
    
    std::string approach;
    std::string target;
    SheepScript* script = nullptr;
};

class NVC : public Asset
{
public:
    NVC(std::string name, char* data, int dataLength);
    
private:
    void ParseFromData(char* data, int dataLength);
    
    // Mapping of noun name to NVC items.
    std::unordered_map<std::string, std::vector<NVCItem>> mNounToItems;
    
    // Mapping of case name to sheep script to eval.
    std::unordered_map<std::string, SheepScript*> mCaseToSheep;
};
