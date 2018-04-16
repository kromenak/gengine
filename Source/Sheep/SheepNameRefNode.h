//
//  SheepNameRefNode.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#pragma once
#include "SheepNode.h"

class SheepNameRefNode : public SheepNode
{
public:
    SheepNameRefNode(const char* name, bool isSystem);
    
private:
    const char* mName;
};
