//
//  SheepScript.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//

#pragma once

#include "SheepNode.h"

class SheepScript
{
public:
    static SheepScript* Create(SheepNode* rootNode);
    
private:
    SheepScript(SheepNode* rootNode);
    
    // The root node of the script - either the symbols or code sections.
    SheepNode* mRootNode = nullptr;
};

