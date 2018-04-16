//
//  SheepNode.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#pragma once

enum class SheepOperationType
{
    None = -1,
    Not = 0,
    Negate,
    Add,
    Subtract,
    Multiply,
    Divide,
    LessThan,
    GreaterThan,
    LessThanOrEqual,
    GreaterThanOrEqual,
    AreEqual,
    NotEqual,
    LogicalOr,
    LogicalAnd
};

enum class SheepReferenceType
{
    Int,
    Float,
    String
};

class SheepNode
{
public:
    static SheepNode* CreateOperation(SheepOperationType operationType);
    static SheepNode* CreateIntegerConstant(int val);
    static SheepNode* CreateFloatConstant(float val);
    static SheepNode* CreateStringConstant(const char* val);
    static SheepNode* CreateTypeRef(SheepReferenceType type);
    static SheepNode* CreateNameRef(const char* name, bool isSystem);
    
    void SetChild(int index, SheepNode* node);
    
    void SetSibling(SheepNode* sibling);
    
protected:
    SheepNode();
    SheepNode(SheepOperationType operationType);
    SheepNode(int constant);
    SheepNode(float constant);
    SheepNode(const char* constant);
    
private:
    union
    {
        int mIntVal;
        float mFloatVal;
    };
    const char* mStringVal = nullptr;
    
    SheepOperationType mOperationType = SheepOperationType::None;
    
    SheepNode* mSibling = nullptr;
    SheepNode* mChildren[2];
};
