//
// SheepScriptBuilder.cpp
//
// Clark Kromenaker
//
#include "SheepScriptBuilder.h"

#include <iostream>

#include "SheepAPI.h"
#include "StringUtil.h"

//#define DEBUG_BUILDER

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
        
        #ifdef DEBUG_BUILDER
        std::cout << "String Const \"" << str << "\"" << std::endl;
        #endif
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
    
    #ifdef DEBUG_BUILDER
    std::cout << "Int Var " << name << " = " << defaultValue << std::endl;
    #endif
    return true;
}

bool SheepScriptBuilder::AddFloatVariable(std::string name, float defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Float;
    sheepValue.floatValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    #ifdef DEBUG_BUILDER
    std::cout << "Float Var " << name << " = " << defaultValue << std::endl;
    #endif
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
    
    #ifdef DEBUG_BUILDER
    std::cout << "String Var " << name << " = \"" << defaultValue << "\"" << std::endl;
    #endif
    return true;
}

void SheepScriptBuilder::StartFunction(std::string functionName)
{
    #ifdef DEBUG_BUILDER
    std::cout << "Function " << functionName << " (" << mBytecode.size() << ")" <<  std::endl;
    #endif
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
    
    #ifdef DEBUG_BUILDER
    std::cout << "End Function " << functionName << std::endl;
    #endif
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
    #ifdef DEBUG_BUILDER
    std::cout << "SitnSpin" << std::endl;
    #endif
    AddInstruction(SheepInstruction::SitnSpin);
}

void SheepScriptBuilder::Yield()
{
    #ifdef DEBUG_BUILDER
    std::cout << "Yield" << std::endl;
    #endif
    AddInstruction(SheepInstruction::Yield);
}

SheepValueType SheepScriptBuilder::CallSysFunction(std::string sysFuncName)
{
    #ifdef DEBUG_BUILDER
    std::cout << "SysFunc " << sysFuncName << std::endl;
    #endif
	
	// All possible system functions are pre-registered in a giant list.
	// So, to compile a system call, we just iterate over and find the matching function.
    for(auto& sysFunc : sysFuncs)
    {
		if(StringUtil::EqualsIgnoreCase(sysFunc.name, sysFuncName))
        {
            // Right before calling the function, we want to push the argument
            // count onto the top of the stack.
            PushI((int)sysFunc.argumentTypes.size());
            
            // Add appropriate instruction based on function return type.
            SheepValueType valueType;
            if(sysFunc.returnType == 0)
            {
                #ifdef DEBUG_BUILDER
                std::cout << "CallSysFunctionV" << std::endl;
                #endif
                AddInstruction(SheepInstruction::CallSysFunctionV);
                valueType = SheepValueType::Void;
            }
            else if(sysFunc.returnType == 1)
            {
                #ifdef DEBUG_BUILDER
                std::cout << "CallSysFunctionI" << std::endl;
                #endif
                AddInstruction(SheepInstruction::CallSysFunctionI);
                valueType = SheepValueType::Int;
            }
            else if(sysFunc.returnType == 2)
            {
                #ifdef DEBUG_BUILDER
                std::cout << "CallSysFunctionF" << std::endl;
                #endif
                AddInstruction(SheepInstruction::CallSysFunctionF);
                valueType = SheepValueType::Float;
            }
            else if(sysFunc.returnType == 3)
            {
                #ifdef DEBUG_BUILDER
                std::cout << "CallSysFunctionS" << std::endl;
                #endif
                AddInstruction(SheepInstruction::CallSysFunctionS);
                valueType = SheepValueType::String;
            }
            else
            {
                std::cout << "Invalid Return Type!" << std::endl;
                return SheepValueType::Void;
            }
            
            // The argument for CallSysFunctionX is the index of the system function
            // that is to be called. So, find the index or use list size by default
            int sysFuncIndex = (int)mSysImports.size();
            for(int i = 0; i < mSysImports.size(); i++)
            {
                if(mSysImports[i].name == sysFuncName)
                {
                    sysFuncIndex = i;
                    break;
                }
            }
            AddIntArg(sysFuncIndex);
            
            // Add sys func to imports list, if not already present.
            if(sysFuncIndex == mSysImports.size())
            {
                mSysImports.push_back(sysFunc);
            }
            
            // We may also need to do a pop for SysFunctionV calls.
            if(valueType == SheepValueType::Void)
            {
                #ifdef DEBUG_BUILDER
                std::cout << "Pop" << std::endl;
                #endif
                AddInstruction(SheepInstruction::Pop);
            }
            return valueType;
        }
    }
    
    // Default - couldn't find this function, so it's just a void I guess.
	std::cout << "Sheep Builder: SysFunc " << sysFuncName << " is not defined! Ignoring." << std::endl;
    return SheepValueType::Void;
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
    #ifdef DEBUG_BUILDER
    std::cout << "BeginWait" << std::endl;
    #endif
    AddInstruction(SheepInstruction::BeginWait);
}

void SheepScriptBuilder::EndWait()
{
    #ifdef DEBUG_BUILDER
    std::cout << "EndWait" << std::endl;
    #endif
    AddInstruction(SheepInstruction::EndWait);
}

void SheepScriptBuilder::ReturnV()
{
    #ifdef DEBUG_BUILDER
    std::cout << "ReturnV" << std::endl;
    #endif
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

SheepValueType SheepScriptBuilder::Load(std::string varName)
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
            return value.type;
        }
    }
    
    // Default return type is void; this is an error!
    return SheepValueType::Void;
}

void SheepScriptBuilder::PushI(int arg)
{
    #ifdef DEBUG_BUILDER
    std::cout << "PushI, " << arg << std::endl;
    #endif
    AddInstruction(SheepInstruction::PushI);
    AddIntArg(arg);
}

void SheepScriptBuilder::PushF(float arg)
{
    #ifdef DEBUG_BUILDER
    std::cout << "PushF, " << arg << std::endl;
    #endif
    AddInstruction(SheepInstruction::PushF);
    AddFloatArg(arg);
}

void SheepScriptBuilder::PushS(std::string arg)
{
    StringUtil::RemoveQuotes(arg);
    
    int offset = GetStringConstOffset(arg);
    if(offset >= 0)
    {
        #ifdef DEBUG_BUILDER
        std::cout << "PushS, " << offset << std::endl;
        #endif
        AddInstruction(SheepInstruction::PushS);
        AddIntArg(offset);
        
        #ifdef DEBUG_BUILDER
        std::cout << "GetString" << std::endl;
        #endif
        AddInstruction(SheepInstruction::GetString);
    }
}

// float plus int = float
// int plus float = float
// float minus int = float
// int minus float = float
// float multiply int = float
// int multiply float = float
// float divide int = float
// int divide float = float

SheepValueType SheepScriptBuilder::Add(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::AddF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::AddF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::AddF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::AddI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::Subtract(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::SubtractF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::SubtractF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::SubtractF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::SubtractI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::Multiply(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::MultiplyF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::MultiplyF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::MultiplyF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::MultiplyI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::Divide(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::DivideF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::DivideF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::DivideF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::DivideI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

void SheepScriptBuilder::Negate(SheepValue val)
{
    if(val.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::NegateI);
    }
    else if(val.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::NegateF);
    }
    else
    {
        // Unsupported
    }
}

SheepValueType SheepScriptBuilder::IsEqual(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsEqualI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::IsNotEqual(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsNotEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsNotEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsNotEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsNotEqualI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::IsGreater(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsGreaterF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsGreaterF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsGreaterF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsGreaterI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::IsLess(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsLessF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsLessF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsLessF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsLessI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::IsGreaterEqual(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsGreaterEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsGreaterEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsGreaterEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsGreaterEqualI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

SheepValueType SheepScriptBuilder::IsLessEqual(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        IToF(1);
        AddInstruction(SheepInstruction::IsLessEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        IToF(0);
        AddInstruction(SheepInstruction::IsLessEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        AddInstruction(SheepInstruction::IsLessEqualF);
        return SheepValueType::Float;
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::IsLessEqualI);
        return SheepValueType::Int;
    }
    else
    {
        // Unsupported
        return SheepValueType::Void;
    }
}

void SheepScriptBuilder::IToF(int index)
{
    AddInstruction(SheepInstruction::IToF);
    AddIntArg(index);
}

void SheepScriptBuilder::FToI(int index)
{
    AddInstruction(SheepInstruction::FToI);
    AddIntArg(index);
}

void SheepScriptBuilder::Modulo(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        FToI(0);
        AddInstruction(SheepInstruction::Modulo);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        FToI(1);
        AddInstruction(SheepInstruction::Modulo);
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        FToI(0);
        FToI(1);
        AddInstruction(SheepInstruction::Modulo);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::Modulo);
    }
    else
    {
        // Unsupported
    }
}

void SheepScriptBuilder::And(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        FToI(0);
        AddInstruction(SheepInstruction::And);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        FToI(1);
        AddInstruction(SheepInstruction::And);
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        FToI(0);
        FToI(1);
        AddInstruction(SheepInstruction::And);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::And);
    }
    else
    {
        // Unsupported
    }
}

void SheepScriptBuilder::Or(SheepValue val1, SheepValue val2)
{
    if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Int)
    {
        FToI(0);
        AddInstruction(SheepInstruction::Or);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Float)
    {
        FToI(1);
        AddInstruction(SheepInstruction::Or);
    }
    else if(val1.type == SheepValueType::Float && val2.type == SheepValueType::Float)
    {
        FToI(0);
        FToI(1);
        AddInstruction(SheepInstruction::Or);
    }
    else if(val1.type == SheepValueType::Int && val2.type == SheepValueType::Int)
    {
        AddInstruction(SheepInstruction::Or);
    }
    else
    {
        // Unsupported
    }
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

