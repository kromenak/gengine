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

//#define SHEEP_DEBUG

SheepVM* SheepVM::sCurrent = nullptr;

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

std::function<void()> SheepVM::GetWaitCallback()
{
	if(mInWaitBlock)
	{
		std::cout << "Increment wait count!" << std::endl;
		mWaitCount++;
		return std::bind(&SheepVM::OnWaitCallback, this);
	}
	return nullptr;
}

void SheepVM::Execute(SheepScript* script, int bytecodeOffset)
{
    if(script == nullptr) { return; }
	mSheepScript = script;
	sCurrent = this;
    
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
				#ifdef SHEEP_DEBUG
				std::cout << "SitnSpin" << std::endl;
				#endif
                break;
            }
            case SheepInstruction::Yield:
            {
                // ???
				#ifdef SHEEP_DEBUG
				std::cout << "Yield" << std::endl;
				#endif
                break;
            }
            case SheepInstruction::CallSysFunctionV:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
				if(sysFunc == nullptr)
				{
					std::cout << "Invalid function index " << functionIndex << std::endl;
					break;
				}
				
				#ifdef SHEEP_DEBUG
				std::cout << "CallSysFuncV " << sysFunc->name << std::endl;
				#endif
                CallSysFunc(sysFunc);
				// Nothing pushed onto stack, since it was a void return.
                break;
            }
            case SheepInstruction::CallSysFunctionI:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
				if(sysFunc == nullptr)
				{
					std::cout << "Invalid function index " << functionIndex << std::endl;
					break;
				}
				
				#ifdef SHEEP_DEBUG
				std::cout << "CallSysFuncI " << sysFunc->name << std::endl;
				#endif
                Value value = CallSysFunc(sysFunc);
				
				// Push the int result onto the stack.
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = value.to<int>();
                break;
            }
            case SheepInstruction::CallSysFunctionF:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
				if(sysFunc == nullptr)
				{
					std::cout << "Invalid function index " << functionIndex << std::endl;
					break;
				}
				
				#ifdef SHEEP_DEBUG
				std::cout << "CallSysFuncF " << sysFunc->name << std::endl;
				#endif
                Value value = CallSysFunc(sysFunc);
				
				// Push the float result onto the stack.
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = value.to<float>();
                break;
            }
            case SheepInstruction::CallSysFunctionS:
            {
                int functionIndex = reader.ReadInt();
                SysImport* sysFunc = script->GetSysImport(functionIndex);
				if(sysFunc == nullptr)
				{
					std::cout << "Invalid function index " << functionIndex << std::endl;
					break;
				}
				
				#ifdef SHEEP_DEBUG
				std::cout << "CallSysFuncS " << sysFunc->name << std::endl;
				#endif
                Value value = CallSysFunc(sysFunc);
				
				// Push the string result onto the stack.
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::String;
                mStack[mStackSize - 1].stringValue = value.to<std::string>().c_str();
                break;
            }
            case SheepInstruction::Branch:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "Branch" << std::endl;
				#endif
				int branchAddress = reader.ReadInt();
				reader.Seek(branchAddress);
                break;
            }
            case SheepInstruction::BranchGoto:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "BranchGoto" << std::endl;
				#endif
				int branchAddress = reader.ReadInt();
				reader.Seek(branchAddress);
                break;
            }
            case SheepInstruction::BranchIfZero:
            {
				// Regardless of whether we do branch, we need to pull
				// the branch address from the reader.
				int branchAddress = reader.ReadInt();
				
				#ifdef SHEEP_DEBUG
				std::cout << "BranchIfZero" << std::endl;
				#endif
				
				// If top item on stack is zero, we will branch.
				if(mStack[mStackSize - 1].intValue == 0)
				{
					reader.Seek(branchAddress);
				}
				
				// This operation also pops from the stack.
				mStackSize--;
                break;
            }
            case SheepInstruction::BeginWait:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "BeginWait" << std::endl;
				#endif
				mInWaitBlock = true;
				mWaitCount = 0;
                break;
            }
            case SheepInstruction::EndWait:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "EndWait " << mInWaitBlock << ", " << mWaitCount << std::endl;
				#endif
				if(mInWaitBlock && mWaitCount > 0)
				{
					mContinueAtOffset = reader.GetPosition();
					return;
				}
                break;
            }
            case SheepInstruction::ReturnV:
            {
                // This means we've reached the end of the executing function.
                // So, we just return to the caller, for realz.
				#ifdef SHEEP_DEBUG
				std::cout << "ReturnV" << std::endl;
				#endif
                return;
            }
            case SheepInstruction::StoreI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreI " << mStack[mStackSize - 1].intValue << std::endl;
					#endif
					
                    assert(mVariables[varIndex].type == SheepValueType::Int);
                    mVariables[varIndex].intValue = mStack[mStackSize - 1].intValue;
					mStackSize--;
                }
                break;
            }
            case SheepInstruction::StoreF:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreF " << mStack[mStackSize - 1].floatValue << std::endl;
					#endif
					
                    assert(mVariables[varIndex].type == SheepValueType::Float);
                    mVariables[varIndex].floatValue = mStack[mStackSize - 1].floatValue;
					mStackSize--;
                }
                break;
            }
            case SheepInstruction::StoreS:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreS " << mStack[mStackSize - 1].stringValue << std::endl;
					#endif
					
                    assert(mVariables[varIndex].type == SheepValueType::String);
                    mVariables[varIndex].stringValue = mStack[mStackSize - 1].stringValue;
					mStackSize--;
                }
                break;
            }
            case SheepInstruction::LoadI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "LoadI " << mVariables[varIndex].intValue << std::endl;
					#endif
					
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
					#ifdef SHEEP_DEBUG
					std::cout << "LoadF " << mVariables[varIndex].floatValue << std::endl;
					#endif
					
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
					#ifdef SHEEP_DEBUG
					std::cout << "LoadS " << mVariables[varIndex].stringValue << std::endl;
					#endif
					
                    assert(mVariables[varIndex].type == SheepValueType::String);
                    mStackSize++;
                    mStack[mStackSize - 1].stringValue = mVariables[varIndex].stringValue;
                }
                break;
            }
            case SheepInstruction::PushI:
            {
                int int1 = reader.ReadInt();
				#ifdef SHEEP_DEBUG
				std::cout << "PushI " << int1 << std::endl;
				#endif
				
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1;
                break;
            }
            case SheepInstruction::PushF:
            {
                float float1 = reader.ReadFloat();
				#ifdef SHEEP_DEBUG
				std::cout << "PushF " << float1 << std::endl;
				#endif
				
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1;
                break;
            }
            case SheepInstruction::PushS:
            {
                int stringConstOffset = reader.ReadInt();
				#ifdef SHEEP_DEBUG
				std::cout << "PushS " << stringConstOffset << std::endl;
				#endif
				
                mStackSize++;
                mStack[mStackSize - 1].type = SheepValueType::String;
                mStack[mStackSize - 1].intValue = stringConstOffset;
                break;
            }
			case SheepInstruction::GetString:
			{
				mStack[mStackSize - 1].type = SheepValueType::String;
				
				std::string* stringPtr = script->GetStringConst(mStack[mStackSize - 1].intValue);
				if(stringPtr != nullptr)
				{
					mStack[mStackSize - 1].stringValue = stringPtr->c_str();
				}
				#ifdef SHEEP_DEBUG
				std::cout << "GetString " << mStack[mStackSize - 1].stringValue << std::endl;
				#endif
				break;
			}
            case SheepInstruction::Pop:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "Pop" << std::endl;
				#endif
                if(mStackSize > 0) { mStackSize--; }
                break;
            }
            case SheepInstruction::AddI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "AddI " << int1 << " + " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 + int2;
                break;
            }
            case SheepInstruction::AddF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "AddF " << float1 << " + " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 + float2;
                break;
            }
            case SheepInstruction::SubtractI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "SubtractI " << int1 << " - " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 - int2;
                break;
            }
            case SheepInstruction::SubtractF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "SubtractF " << float1 << " - " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 - float2;
                break;
            }
            case SheepInstruction::MultiplyI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "MultiplyI " << int1 << " * " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 * int2;
                break;
            }
            case SheepInstruction::MultiplyF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "MultiplyF " << float1 << " * " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 * float2;
                break;
            }
            case SheepInstruction::DivideI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "DivideI " << int1 << " / " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 / int2;
                break;
            }
            case SheepInstruction::DivideF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "DivideF " << float1 << " / " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Float;
                mStack[mStackSize - 1].floatValue = float1 / float2;
                break;
            }
            case SheepInstruction::NegateI:
            {
                if(mStackSize < 1) { break; }
				
				#ifdef SHEEP_DEBUG
				std::cout << "NegateI " << mStack[mStackSize - 1].intValue << std::endl;
				#endif
                mStack[mStackSize - 1].intValue *= -1;
                break;
            }
            case SheepInstruction::NegateF:
            {
                if(mStackSize < 1) { break; }
				
				#ifdef SHEEP_DEBUG
				std::cout << "NegateF " << mStack[mStackSize - 1].floatValue << std::endl;
				#endif
                mStack[mStackSize - 1].floatValue *= -1.0f;
                break;
            }
            case SheepInstruction::IsEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsEqualI " << int1 << " == " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 == int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsEqualF " << float1 << " == " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsNotEqualI " << int1 << " != " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 != int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsNotEqualF " << float1 << " != " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (!Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterI " << int1 << " > " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 > int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterF " << float1 << " > " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (float1 > float2 ? 1 : 0);
                break;
            }
			case SheepInstruction::IsLessI:
			{
				if(mStackSize < 2) { break; }
				
				int int1 = mStack[mStackSize - 2].intValue;
				int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessI " << int1 << " < " << int2 << std::endl;
				#endif
				
				mStackSize--;
				mStack[mStackSize - 1].type = SheepValueType::Int;
				mStack[mStackSize - 1].intValue = (int1 < int2 ? 1 : 0);
				break;
			}
			case SheepInstruction::IsLessF:
			{
				if(mStackSize < 2) { break; }
				
				float float1 = mStack[mStackSize - 2].floatValue;
				float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessF " << float1 << " < " << float2 << std::endl;
				#endif
				
				mStackSize--;
				mStack[mStackSize - 1].type = SheepValueType::Int;
				mStack[mStackSize - 1].intValue = (float1 < float2 ? 1 : 0);
				break;
			}
            case SheepInstruction::IsGreaterEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterEqualI " << int1 << " >= " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 >= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterEqualF " << float1 << " >= " << float2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (float1 >= float2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessEqualI " << int1 << " <= " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 <= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessEqualF " << float1 << " <= " << float2 << std::endl;
				#endif
				
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
				
				#ifdef SHEEP_DEBUG
				std::cout << "IToF " << mStack[stackIndex].intValue << std::endl;
				#endif
                mStack[stackIndex].floatValue = mStack[stackIndex].intValue;
                mStack[stackIndex].type = SheepValueType::Float;
                break;
            }
            case SheepInstruction::FToI:
            {
                int indexFromTop = reader.ReadInt();
                int stackIndex = mStackSize - 1 - indexFromTop;
                if(stackIndex < 0 || stackIndex >= mStackSize) { break; }
				
				#ifdef SHEEP_DEBUG
				std::cout << "FToI " << mStack[stackIndex].floatValue << std::endl;
				#endif
                mStack[stackIndex].intValue = mStack[stackIndex].floatValue;
                mStack[stackIndex].type = SheepValueType::Int;
                break;
            }
            case SheepInstruction::Modulo:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "Modulo " << int1 << " % " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = int1 % int2;
                break;
            }
            case SheepInstruction::And:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "And " << int1 << " && " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 && int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::Or:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "Or " << int1 << " || " << int2 << std::endl;
				#endif
				
                mStackSize--;
                mStack[mStackSize - 1].type = SheepValueType::Int;
                mStack[mStackSize - 1].intValue = (int1 || int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::Not:
            {
                if(mStackSize < 1) { break; }
				
				int int1 = mStack[mStackSize - 1].intValue;
				#ifdef SHEEP_DEBUG
				std::cout << "Not " << int1 << std::endl;
				#endif
				
                mStack[mStackSize - 1].intValue = (int1 == 0 ? 1 : 0);
                break;
            }
            case SheepInstruction::DebugBreakpoint:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "DebugBreakpoint" << std::endl;
				#endif
                break;
            }
            default:
            {
				std::cout << "Unaccounted for Sheep Instruction: " << (int)instruction << std::endl;
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
		case 0:
			return ::CallSysFunc(sysFunc->name);
        case 1:
            return ::CallSysFunc(sysFunc->name, args[0]);
        case 2:
            return ::CallSysFunc(sysFunc->name, args[0], args[1]);
		case 3:
			return ::CallSysFunc(sysFunc->name, args[0], args[1], args[2]);
		case 4:
			return ::CallSysFunc(sysFunc->name, args[0], args[1], args[2], args[3]);
		case 5:
			return ::CallSysFunc(sysFunc->name, args[0], args[1], args[2], args[3], args[4]);
		case 6:
			return ::CallSysFunc(sysFunc->name, args[0], args[1], args[2], args[3], args[4], args[5]);
        default:
            std::cout << "SheepVM: Unimplemented arg count: " << argCount << std::endl;
            return Value(0);
    }
}

void SheepVM::OnWaitCallback()
{
	assert(mInWaitBlock);
	assert(mWaitCount > 0);
	
	mWaitCount--;
	if(mWaitCount == 0)
	{
		std::cout << "Done waiting! Resuming Sheep execution at position " << mContinueAtOffset << std::endl;
		Execute(mSheepScript, mContinueAtOffset);
	}
}
