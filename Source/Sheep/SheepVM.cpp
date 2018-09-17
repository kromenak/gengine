//
// SheepVM.cpp
//
// Clark Kromenaker
//
#include "SheepVM.h"

#include <iostream>

#include "BinaryReader.h"
#include "Math.h"
#include "SheepAPI.h"
#include "SheepScript.h"

void SheepVM::Execute(SheepScript* script)
{
    // We need a valid script.
    if(script == nullptr) { return; }
    
    // Just default to zero offset (aka the first function in the script).
    Execute(script, 0);
}

void SheepVM::Execute(SheepScript* script, std::string functionName)
{
    // We need a valid script.
    if(script == nullptr) { return; }

    // Get bytecode offset for this function. If less than zero,
    // it means the function doesn't exist, and we've got to fail out.
    int functionOffset = script->GetFunctionOffset(functionName);
    if(functionOffset < 0) { return; }
    
    // Execute that function.
    Execute(script, functionOffset);
}

bool SheepVM::Evaluate(SheepScript* script)
{
    // Execute the script, per usual.
    Execute(script);
    
    // If stack is empty, return false.
    if(mStackSize == 0) { return false; }
    
    // Check the top item on the stack and return true or false based on that.
    if(GetStack(0).type == SheepValueType::Int)
    {
        return GetStack(0).intValue != 0;
    }
    else if(GetStack(0).type == SheepValueType::Float)
    {
        return !Math::AreEqual(GetStack(0).floatValue, 0.0f);
    }
    else if(GetStack(0).type == SheepValueType::String)
    {
        std::string str(GetStack(0).stringValue);
        return !str.empty();
    }

    // Default to false.
    return false;
}

void SheepVM::Execute(SheepScript *script, int bytecodeOffset)
{
    if(script == nullptr) { return; }
    
    // Create copy of variables for assignment during execution.
    mVariables = script->GetVariables();
    
    // Get bytecode and generate a binary reader for easier parsing.
    char* bytecode = script->GetBytecode();
    int bytecodeLength = script->GetBytecodeLength();
    
    // Create reader for the bytecode.
    BinaryReader reader(bytecode, bytecodeLength);
    if(!reader.CanRead()) { return; }
    
    // Skip ahead to desired offset.
    reader.Skip(bytecodeOffset);
    
    // Read each byte in turn, interpret and execute the instruction.
    while(!reader.IsEof())
    {
        char instruction = reader.ReadUByte();
        switch((SheepInstruction)instruction)
        {
            case SheepInstruction::SitnSpin:
            {
                // No-op; do nothing.
                break;
            }
            case SheepInstruction::Yield:
            {
                // ???
                break;
            }
            case SheepInstruction::CallSysFunctionV:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
                CallSysFunc(sysFunc);
                break;
            }
            case SheepInstruction::CallSysFunctionI:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
                Value value = CallSysFunc(sysFunc);
                
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = value.to<int>();
                break;
            }
            case SheepInstruction::CallSysFunctionF:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
                Value value = CallSysFunc(sysFunc);
                
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = value.to<float>();
                break;
            }
            case SheepInstruction::CallSysFunctionS:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
                Value value = CallSysFunc(sysFunc);
                
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::String;
                mStack[mStackSize - 1].stringValue = value.to<std::string>().c_str();
                break;
            }
            case SheepInstruction::Branch:
            {
                break;
            }
            case SheepInstruction::BranchGoto:
            {
                break;
            }
            case SheepInstruction::BranchIfZero:
            {
                break;
            }
            case SheepInstruction::BeginWait:
            {
                break;
            }
            case SheepInstruction::EndWait:
            {
                break;
            }
            case SheepInstruction::ReturnV:
            {
                // This means we've reached the end of the executing function.
                // We should return!
                return;
            }
            case SheepInstruction::StoreI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::Int);
                    mVariables[varIndex].intValue = mStack[mStackSize - 1].intValue;
                }
                break;
            }
            case SheepInstruction::StoreF:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::Float);
                    mVariables[varIndex].floatValue = mStack[mStackSize - 1].floatValue;
                }
                break;
            }
            case SheepInstruction::StoreS:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::String);
                    mVariables[varIndex].stringValue = mStack[mStackSize - 1].stringValue;
                }
                break;
            }
            case SheepInstruction::LoadI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::Int);
                    mStackSize++;
                    mStack[mStackSize - 1].intValue = mVariables[varIndex].intValue;
                }
                break;
            }
            case SheepInstruction::LoadF:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::Float);
                    mStackSize++;
                    mStack[mStackSize - 1].floatValue = mVariables[varIndex].floatValue;
                }
                break;
            }
            case SheepInstruction::LoadS:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
                    assert(mVariables[varIndex].type == SheepValueType::String);
                    mStackSize++;
                    mStack[mStackSize - 1].stringValue = mVariables[varIndex].stringValue;
                }
                break;
            }
            case SheepInstruction::PushI:
            {
                int int1 = reader.ReadInt();
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1;
                break;
            }
            case SheepInstruction::PushF:
            {
                float float1 = reader.ReadFloat();
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1;
                break;
            }
            case SheepInstruction::PushS:
            {
                int stringConstOffset = reader.ReadInt();
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::String;
                mStack[mStackSize - 1].intValue = stringConstOffset;
                break;
            }
            case SheepInstruction::Pop:
            {
                if(mStackSize > 0) { mStackSize--; }
                break;
            }
            case SheepInstruction::AddI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 + int2;
                break;
            }
            case SheepInstruction::AddF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 + float2;
                break;
            }
            case SheepInstruction::SubtractI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 - int2;
                break;
            }
            case SheepInstruction::SubtractF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 - float2;
                break;
            }
            case SheepInstruction::MultiplyI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 * int2;
                break;
            }
            case SheepInstruction::MultiplyF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 * float2;
                break;
            }
            case SheepInstruction::DivideI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 / int2;
                break;
            }
            case SheepInstruction::DivideF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 / float2;
                break;
            }
            case SheepInstruction::NegateI:
            {
                if(mStackSize < 1) { break; }
                mStack[mStackSize - 1].intValue *= -1;
                break;
            }
            case SheepInstruction::NegateF:
            {
                if(mStackSize < 1) { break; }
                mStack[mStackSize - 1].floatValue *= -1.0f;
                break;
            }
            case SheepInstruction::IsEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 == int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 != int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (!Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 > int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (float1 > float2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 >= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (float1 >= float2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 <= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 1].floatValue;
                float float2 = mStack[mStackSize - 2].floatValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (float1 <= float2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IToF:
            {
                int indexFromTop = reader.ReadInt();
                int stackIndex = mStackSize - 1 - indexFromTop;
                if(stackIndex < 0 || stackIndex >= mStackSize) { break; }
                mStack[stackIndex].floatValue = mStack[stackIndex].intValue;
                mStack[stackIndex].type = SheepValueType::Float;
                break;
            }
            case SheepInstruction::FToI:
            {
                int indexFromTop = reader.ReadInt();
                int stackIndex = mStackSize - 1 - indexFromTop;
                if(stackIndex < 0 || stackIndex >= mStackSize) { break; }
                mStack[stackIndex].intValue = mStack[stackIndex].floatValue;
                mStack[stackIndex].type = SheepValueType::Int;
                break;
            }
            case SheepInstruction::Modulo:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 % int2;
                break;
            }
            case SheepInstruction::And:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 && int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::Or:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 1].intValue;
                int int2 = mStack[mStackSize - 2].intValue;
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 || int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::Not:
            {
                if(mStackSize < 1) { break; }
                int int1 = mStack[mStackSize - 1].intValue;
                mStack[mStackSize - 1].intValue = (int1 == 0 ? 1 : 0);
                break;
            }
            case SheepInstruction::GetString:
            {
                mStack[mStackSize - 1].type = SheepValueType::String;
                mStack[mStackSize - 1].stringValue = script->GetStringConst(mStack[mStackSize - 1].intValue).c_str();
                break;
            }
            case SheepInstruction::DebugBreakpoint:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

Value SheepVM::CallSysFunc(SysImport* sysFunc)
{
    // Number on top of stack is argument count.
    int argCount = GetStack(0).intValue;
    mStackSize--;
    
    //TODO: Make sure SysFunc arg count matches the arg count on the stack.
    
    std::vector<Value> args;
    for(int i = 0; i < argCount; i++)
    {
        SheepValue sheepValue = mStack[mStackSize - (argCount - i)];
        
        std::string str;
        int argType = sysFunc->argumentTypes[i];
        switch(argType)
        {
            case 1:
                args.push_back(sheepValue.intValue);
                break;
            case 2:
                args.push_back(sheepValue.floatValue);
                break;
            case 3:
                str = std::string(sheepValue.stringValue);
                args.push_back(std::string(sheepValue.stringValue));
                break;
            default:
                std::cout << "Invalid arg type: " << argType << std::endl;
                break;
        }
    }
    mStackSize -= argCount;
    
    switch(argCount)
    {
        case 1:
            return ::CallSysFunc(sysFunc->name, args[0]);
        case 2:
            return ::CallSysFunc(sysFunc->name, args[0], args[1]);
        default:
            std::cout << "SheepVM: Unimplemented arg count: " << argCount << std::endl;
            return Value(0);
    }
}
