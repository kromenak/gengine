//
//  SheepScript.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//

#include "SheepScript.h"

SheepScript* gCurrentSheepScript = nullptr;

SheepScript* SheepScript::Create(SheepNode *rootNode)
{
    return new SheepScript(rootNode);
}

SheepScript::SheepScript(SheepNode* rootNode) : mRootNode(rootNode)
{

}
