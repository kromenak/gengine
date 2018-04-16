//
//  SheepTypeRefNode.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#pragma once
#include "SheepNode.h"

class SheepTypeRefNode : public SheepNode
{
public:
    SheepTypeRefNode(SheepReferenceType referenceType);
    
private:
    SheepReferenceType mSheepReferenceType;
};
