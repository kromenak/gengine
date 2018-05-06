//
// SheepScriptBuilder.cpp
//
// Clark Kromenaker
//
#include "SheepScriptBuilder.h"
#include <iostream>
#include "StringUtil.h"

bool SheepScriptBuilder::AddSysFuncImport(std::string name)
{
    return false;
}

bool SheepScriptBuilder::AddStringConst(std::string str)
{
    StringUtil::RemoveQuotes(str);
    auto it = std::find(mStringConsts.begin(), mStringConsts.end(), str);
    if(it == mStringConsts.end())
    {
        mStringConsts.push_back(str);
        std::cout << "Added String Const " << str << std::endl;
        return true;
    }
    return false;
}

bool SheepScriptBuilder::AddIntVariable(std::string name, int defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Int;
    sheepValue.intValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    std::cout << "Added int variable " << name << " with default value " << defaultValue << std::endl;
    return true;
}

bool SheepScriptBuilder::AddFloatVariable(std::string name, float defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Float;
    sheepValue.floatValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    std::cout << "Added float variable " << name << " with default value " << defaultValue << std::endl;
    return true;
}

bool SheepScriptBuilder::AddStringVariable(std::string name, std::string defaultValue)
{
    AddStringConst(defaultValue);
    
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::String;
    sheepValue.stringValue = defaultValue.c_str();
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    std::cout << "Added string variable " << name << " with default value " << defaultValue << std::endl;
    return true;
}

bool SheepScriptBuilder::AddFunction(std::string functionName)
{
    mFunctions[functionName] = mLastFunctionIndex;
    std::cout << "Added function at bytecode offset " << mLastFunctionIndex << std::endl;
    mLastFunctionIndex = (int)mBytecode.size();
    return true;
}

void SheepScriptBuilder::SitNSpin()
{
    AddInstruction(SheepInstruction::SitNSpin);
}

void SheepScriptBuilder::Yield()
{
    AddInstruction(SheepInstruction::Yield);
}

void SheepScriptBuilder::BeginWait()
{
    AddInstruction(SheepInstruction::BeginWait);
}

void SheepScriptBuilder::EndWait()
{
    AddInstruction(SheepInstruction::EndWait);
}

void SheepScriptBuilder::ReturnV()
{
    AddInstruction(SheepInstruction::ReturnV);
}

// Store

void SheepScriptBuilder::Load(std::string varName)
{
    auto it = mVariableIndexByName.find(varName);
    if(it != mVariableIndexByName.end())
    {
        int varIndex = it->second;
        if(varIndex >= 0 && varIndex < mVariables.size())
        {
            SheepValue& value = mVariables[varIndex];
            if(value.type == SheepValueType::Int)
            {
                AddInstruction(SheepInstruction::LoadI);
            }
            else if(value.type == SheepValueType::Float)
            {
                AddInstruction(SheepInstruction::LoadF);
            }
            else if(value.type == SheepValueType::String)
            {
                AddInstruction(SheepInstruction::LoadS);
            }
            AddIntArg(varIndex);
        }
    }
}

void SheepScriptBuilder::PushI(int arg)
{
    std::cout << "PushI, " << arg << std::endl;
    AddInstruction(SheepInstruction::PushI);
    AddIntArg(arg);
}

// PushF

void SheepScriptBuilder::PushS(std::string arg)
{
    int offset = GetStringConstOffset(arg);
    if(offset >= 0)
    {
        std::cout << "PushS, " << offset << std::endl;
        AddInstruction(SheepInstruction::PushS);
        AddIntArg(offset);
        AddInstruction(SheepInstruction::GetString);
    }
}

void SheepScriptBuilder::AddI()
{
    AddInstruction(SheepInstruction::AddI);
}

void SheepScriptBuilder::AddF()
{
    AddInstruction(SheepInstruction::AddF);
}

void SheepScriptBuilder::SubtractI()
{
    AddInstruction(SheepInstruction::SubtractI);
}

void SheepScriptBuilder::SubtractF()
{
    AddInstruction(SheepInstruction::SubtractF);
}

void SheepScriptBuilder::MultiplyI()
{
    AddInstruction(SheepInstruction::MultiplyI);
}

void SheepScriptBuilder::MultiplyF()
{
    AddInstruction(SheepInstruction::MultiplyF);
}

void SheepScriptBuilder::DivideI()
{
    AddInstruction(SheepInstruction::DivideI);
}

void SheepScriptBuilder::DivideF()
{
    AddInstruction(SheepInstruction::DivideF);
}

void SheepScriptBuilder::NegateI()
{
    AddInstruction(SheepInstruction::NegateI);
}

void SheepScriptBuilder::NegateF()
{
    AddInstruction(SheepInstruction::NegateF);
}

void SheepScriptBuilder::IsEqualI()
{
    AddInstruction(SheepInstruction::IsEqualI);
}

void SheepScriptBuilder::Modulo()
{
    AddInstruction(SheepInstruction::Modulo);
}

void SheepScriptBuilder::Breakpoint()
{
    AddInstruction(SheepInstruction::DebugBreakpoint);
}

void SheepScriptBuilder::AddInstruction(SheepInstruction instr)
{
    mBytecode.push_back((char)instr);
}

void SheepScriptBuilder::AddIntArg(int arg)
{
    mBytecode.push_back(arg & 0xFF);
    mBytecode.push_back((arg >> 8) & 0xFF);
    mBytecode.push_back((arg >> 16) & 0xFF);
    mBytecode.push_back((arg >> 24) & 0xFF);
}

int SheepScriptBuilder::GetStringConstOffset(std::string stringConst)
{
    // Iterate over all string consts until we find a match for the passed string.
    // We need to keep track of the offset as we go, since that's what we want
    // to return.
    int offset = 0;
    for(auto& str : mStringConsts)
    {
        if(stringConst == str) { return offset; }
        
        // +1 because offset should account for \0 null terminator.
        // Ex: even "" has size of 1!
        offset += str.size() + 1;
    }
    
    // Couldn't find it; return -1 to indicate failure.
    return -1;
}

