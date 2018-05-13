//
// SheepScriptBuilder.cpp
//
// Clark Kromenaker
//
#include "SheepScriptBuilder.h"
#include <iostream>
#include "StringUtil.h"
#include "SheepAPI.h"

SheepScriptBuilder::SheepScriptBuilder()
{
    // Always add the empty string as a constant.
    AddStringConst("");
}

bool SheepScriptBuilder::AddStringConst(std::string str)
{
    StringUtil::RemoveQuotes(str);
    auto it = std::find(mStringConsts.begin(), mStringConsts.end(), str);
    if(it == mStringConsts.end())
    {
        mStringConsts.push_back(str);
        mStringConstsByOffset[mStringConstsOffset] = str;
        mStringConstsOffset += str.size() + 1;
        
        std::cout << "String Const \"" << str << "\"" << std::endl;
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
    
    std::cout << "Int Var " << name << " = " << defaultValue << std::endl;
    return true;
}

bool SheepScriptBuilder::AddFloatVariable(std::string name, float defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Float;
    sheepValue.floatValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    std::cout << "Float Var " << name << " = " << defaultValue << std::endl;
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
    
    std::cout << "String Var " << name << " = \"" << defaultValue << "\"" << std::endl;
    return true;
}

void SheepScriptBuilder::StartFunction(std::string functionName)
{
    std::cout << "Function " << functionName << " (" << mBytecode.size() << ")" <<  std::endl;
    mFunctions[functionName] = (int)mBytecode.size();
}

void SheepScriptBuilder::EndFunction(std::string functionName)
{
    // If the writer didn't explicitly add a return statement at the
    // end of their function, add one implicitly here.
    if(mBytecode.size() == 0 || mBytecode[mBytecode.size() - 1] != 0x0B)
    {
        ReturnV();
    }
    
    // For whatever reason, Sheep functions seem to always end with 4 SitnSpins in a row.
    SitnSpin();
    SitnSpin();
    SitnSpin();
    SitnSpin();
    std::cout << "End Function " << functionName << std::endl;
}

void SheepScriptBuilder::AddGoto(std::string labelName)
{
    // Add the label and record the offset, but don't allow duplicates.
    auto it = mGotoLabelsToOffsets.find(labelName);
    if(it == mGotoLabelsToOffsets.end())
    {
        int gotoOffset = (int)mBytecode.size();
        mGotoLabelsToOffsets[labelName] = gotoOffset;
        
        // Check whether we have any "goto" instructions that used this label before
        // we knew the exact offset. We have to go back and fill in those values.
        auto it2 = mGotoLabelsToBeHookedUp.find(labelName);
        if(it2 != mGotoLabelsToBeHookedUp.end())
        {
            std::vector<int> offsets = it2->second;
            for(auto& offset : offsets)
            {
                mBytecode[offset] = (gotoOffset & 0xFF);
                mBytecode[offset + 1] = (gotoOffset >> 8) & 0xFF;
                mBytecode[offset + 2] = (gotoOffset >> 16) & 0xFF;
                mBytecode[offset + 3] = (gotoOffset >> 24) & 0xFF;
            }
            mGotoLabelsToBeHookedUp.erase(labelName);
        }
    }
    else
    {
        //TODO: Error: duplicate goto label
    }
}

void SheepScriptBuilder::SitnSpin()
{
    std::cout << "SitnSpin" << std::endl;
    AddInstruction(SheepInstruction::SitnSpin);
}

void SheepScriptBuilder::Yield()
{
    std::cout << "Yield" << std::endl;
    AddInstruction(SheepInstruction::Yield);
}

void SheepScriptBuilder::CallSysFunction(std::string sysFuncName)
{
    std::cout << "SysFunc " << sysFuncName << std::endl;
    for(auto& sysFunc : sysFuncs)
    {
        if(sysFunc.name == sysFuncName)
        {
            // Right before calling the function, we want to push the argument
            // count onto the top of the stack.
            PushI((int)sysFunc.argumentTypes.size());
            
            // Add appropriate instruction based on function return type.
            bool doPop = false;
            if(sysFunc.returnType == 0)
            {
                std::cout << "CallSysFunctionV" << std::endl;
                AddInstruction(SheepInstruction::CallSysFunctionV);
                doPop = true;
            }
            else if(sysFunc.returnType == 1)
            {
                std::cout << "CallSysFunctionI" << std::endl;
                AddInstruction(SheepInstruction::CallSysFunctionI);
            }
            else if(sysFunc.returnType == 2)
            {
                std::cout << "CallSysFunctionF" << std::endl;
                AddInstruction(SheepInstruction::CallSysFunctionF);
            }
            else if(sysFunc.returnType == 3)
            {
                std::cout << "CallSysFunctionS" << std::endl;
                AddInstruction(SheepInstruction::CallSysFunctionS);
            }
            
            // The argument for CallSysFunctionX is the index of the system function
            // that is to be called. So, find the index and add that int arg.
            for(int i = 0; i < mSysImports.size(); i++)
            {
                if(mSysImports[i].name == sysFuncName)
                {
                    AddIntArg(i);
                    return;
                }
            }
            
            // If we didn't find it, it means we've encountered a new system function call.
            // Add it to the list and of system function imports and set the int arg.
            AddIntArg((int)mSysImports.size());
            mSysImports.push_back(sysFunc);
            
            // We may also need to do a pop for SysFunctionV calls.
            if(doPop)
            {
                std::cout << "Pop" << std::endl;
                AddInstruction(SheepInstruction::Pop);
            }
        }
    }
}

void SheepScriptBuilder::BranchGoto(std::string labelName)
{
    AddInstruction(SheepInstruction::BranchGoto);
    
    // Argument is the offset in the bytecode to the "goto" label.
    // However...we might already know that offset, or we might not!
    auto it = mGotoLabelsToOffsets.find(labelName);
    if(it != mGotoLabelsToOffsets.end())
    {
        // If we already know the offset, just add the argument and be done with it.
        AddIntArg(it->second);
    }
    else
    {
        // We DO NOT yet know the offset, so just push -1 as a placeholder.
        AddIntArg(-1);
        
        // Keep track of the fact that we need to revisit this argument once we
        // know where the "goto" label is located.
        auto it = mGotoLabelsToBeHookedUp.find(labelName);
        if(it == mGotoLabelsToBeHookedUp.end())
        {
            mGotoLabelsToBeHookedUp[labelName] = std::vector<int>();
        }
        mGotoLabelsToBeHookedUp[labelName].push_back((int)(mBytecode.size() - 4));
    }
}

void SheepScriptBuilder::BeginWait()
{
    std::cout << "BeginWait" << std::endl;
    AddInstruction(SheepInstruction::BeginWait);
}

void SheepScriptBuilder::EndWait()
{
    std::cout << "EndWait" << std::endl;
    AddInstruction(SheepInstruction::EndWait);
}

void SheepScriptBuilder::ReturnV()
{
    std::cout << "ReturnV" << std::endl;
    AddInstruction(SheepInstruction::ReturnV);
}

void SheepScriptBuilder::Store(std::string varName)
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
                AddInstruction(SheepInstruction::StoreI);
            }
            else if(value.type == SheepValueType::Float)
            {
                AddInstruction(SheepInstruction::StoreF);
            }
            else if(value.type == SheepValueType::String)
            {
                AddInstruction(SheepInstruction::StoreS);
            }
            AddIntArg(varIndex);
        }
    }
}

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

void SheepScriptBuilder::PushF(float arg)
{
    std::cout << "PushF, " << arg << std::endl;
    AddInstruction(SheepInstruction::PushF);
    AddFloatArg(arg);
}

void SheepScriptBuilder::PushS(std::string arg)
{
    StringUtil::RemoveQuotes(arg);
    
    int offset = GetStringConstOffset(arg);
    if(offset >= 0)
    {
        std::cout << "PushS, " << offset << std::endl;
        AddInstruction(SheepInstruction::PushS);
        AddIntArg(offset);
        
        std::cout << "GetString" << std::endl;
        AddInstruction(SheepInstruction::GetString);
    }
}

void SheepScriptBuilder::AddI()
{
    std::cout << "AddI" << std::endl;
    AddInstruction(SheepInstruction::AddI);
}

void SheepScriptBuilder::AddF()
{
    std::cout << "AddF" << std::endl;
    AddInstruction(SheepInstruction::AddF);
}

void SheepScriptBuilder::SubtractI()
{
    std::cout << "SubtractI" << std::endl;
    AddInstruction(SheepInstruction::SubtractI);
}

void SheepScriptBuilder::SubtractF()
{
    std::cout << "SubtractF" << std::endl;
    AddInstruction(SheepInstruction::SubtractF);
}

void SheepScriptBuilder::MultiplyI()
{
    std::cout << "MultiplyI" << std::endl;
    AddInstruction(SheepInstruction::MultiplyI);
}

void SheepScriptBuilder::MultiplyF()
{
    std::cout << "MultiplyF" << std::endl;
    AddInstruction(SheepInstruction::MultiplyF);
}

void SheepScriptBuilder::DivideI()
{
    std::cout << "DivideI" << std::endl;
    AddInstruction(SheepInstruction::DivideI);
}

void SheepScriptBuilder::DivideF()
{
    std::cout << "DivideF" << std::endl;
    AddInstruction(SheepInstruction::DivideF);
}

void SheepScriptBuilder::NegateI()
{
    std::cout << "NegateI" << std::endl;
    AddInstruction(SheepInstruction::NegateI);
}

void SheepScriptBuilder::NegateF()
{
    std::cout << "NegateF" << std::endl;
    AddInstruction(SheepInstruction::NegateF);
}

void SheepScriptBuilder::IsEqualI()
{
    AddInstruction(SheepInstruction::IsEqualI);
}

void SheepScriptBuilder::IsEqualF()
{
    AddInstruction(SheepInstruction::IsEqualF);
}

void SheepScriptBuilder::IsNotEqualI()
{
    AddInstruction(SheepInstruction::IsNotEqualI);
}

void SheepScriptBuilder::IsNotEqualF()
{
    AddInstruction(SheepInstruction::IsNotEqualF);
}

void SheepScriptBuilder::IsGreaterI()
{
    AddInstruction(SheepInstruction::IsGreaterI);
}

void SheepScriptBuilder::IsGreaterF()
{
    AddInstruction(SheepInstruction::IsGreaterF);
}

void SheepScriptBuilder::IsLessI()
{
    AddInstruction(SheepInstruction::IsLessI);
}

void SheepScriptBuilder::IsLessF()
{
    AddInstruction(SheepInstruction::IsLessF);
}

void SheepScriptBuilder::IsGreaterEqualI()
{
    AddInstruction(SheepInstruction::IsGreaterEqualI);
}

void SheepScriptBuilder::IsGreaterEqualF()
{
    AddInstruction(SheepInstruction::IsGreaterEqualF);
}

void SheepScriptBuilder::IsLessEqualI()
{
    AddInstruction(SheepInstruction::IsLessEqualI);
}

void SheepScriptBuilder::IsLessEqualF()
{
    AddInstruction(SheepInstruction::IsLessEqualF);
}

//ITOF/FTOI

void SheepScriptBuilder::Modulo()
{
    AddInstruction(SheepInstruction::Modulo);
}

void SheepScriptBuilder::And()
{
    AddInstruction(SheepInstruction::And);
}

void SheepScriptBuilder::Or()
{
    AddInstruction(SheepInstruction::Or);
}

void SheepScriptBuilder::Not()
{
    AddInstruction(SheepInstruction::Not);
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

void SheepScriptBuilder::AddFloatArg(float arg)
{
    char* array;
    array = reinterpret_cast<char*>(&arg);
    mBytecode.push_back(array[0]);
    mBytecode.push_back(array[1]);
    mBytecode.push_back(array[2]);
    mBytecode.push_back(array[3]);
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

