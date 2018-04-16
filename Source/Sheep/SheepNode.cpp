//
//  SheepNode.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#include "SheepNode.h"
#include "SheepTypeRefNode.h"
#include "SheepNameRefNode.h"

SheepNode* SheepNode::CreateOperation(SheepOperationType operationType)
{
    return new SheepNode(operationType);
}

SheepNode* SheepNode::CreateIntegerConstant(int val)
{
    return new SheepNode(val);
}

SheepNode* SheepNode::CreateFloatConstant(float val)
{
    return new SheepNode(val);
}

SheepNode* SheepNode::CreateStringConstant(const char *val)
{
    return new SheepNode(val);
}

SheepNode* SheepNode::CreateTypeRef(SheepReferenceType type)
{
    return new SheepTypeRefNode(type);
}

SheepNode* SheepNode::CreateNameRef(const char *name, bool isSystem)
{
    return new SheepNameRefNode(name, isSystem);
}

SheepNode::SheepNode()
{
    
}

SheepNode::SheepNode(SheepOperationType operationType) :
    mOperationType(operationType)
{
    
}

SheepNode::SheepNode(int constant) : mIntVal(constant)
{
    
}

SheepNode::SheepNode(float constant) : mFloatVal(constant)
{
    
}

SheepNode::SheepNode(const char* constant) : mStringVal(constant)
{
    
}

void SheepNode::SetChild(int index, SheepNode *node)
{
    //TODO: Clamp index!
    mChildren[index] = node;
}

void SheepNode::SetSibling(SheepNode *sibling)
{
    mSibling = sibling;
}
