#include "SheepScriptBuilder.h"

#include <iostream>

#include "SheepCompiler.h"
#include "SheepSysFunc.h"
#include "StringUtil.h"

//#define DEBUG_BUILDER

SheepScriptBuilder::SheepScriptBuilder()
{
	#ifdef DEBUG_BUILDER
	std::cout << "SheepBuilder BEGIN" << std::endl;
	#endif
	
    // Always add the empty string as a constant.
    AddStringConst("");
}

SheepScriptBuilder::~SheepScriptBuilder()
{
	#ifdef DEBUG_BUILDER
	std::cout << "SheepBuilder END" << std::endl;
	#endif
}

void SheepScriptBuilder::AddStringConst(std::string str)
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
    }
}

void SheepScriptBuilder::AddIntVariable(std::string name, int defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Int;
    sheepValue.intValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    #ifdef DEBUG_BUILDER
    std::cout << "Int Var " << name << " = " << defaultValue << std::endl;
    #endif
}

void SheepScriptBuilder::AddFloatVariable(std::string name, float defaultValue)
{
    SheepValue sheepValue;
    sheepValue.type = SheepValueType::Float;
    sheepValue.floatValue = defaultValue;
    
    mVariableIndexByName[name] = (int)mVariables.size();
    mVariables.push_back(sheepValue);
    
    #ifdef DEBUG_BUILDER
    std::cout << "Float Var " << name << " = " << defaultValue << std::endl;
    #endif
}

void SheepScriptBuilder::AddStringVariable(std::string name, std::string defaultValue)
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
}

void SheepScriptBuilder::StartFunction(std::string functionName)
{
    #ifdef DEBUG_BUILDER
    std::cout << "Function " << functionName << " (" << mBytecode.size() << ")" <<  std::endl;
    #endif
	
	// All sheep functions are case-insensitive.
	// So, lower-case the function name in the hash table, so we can lookup consistently.
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

void SheepScriptBuilder::AddGoto(std::string labelName, const Location& loc)
{
    // Add the label and record the offset, but don't allow duplicates.
    auto it = mGotoLabelsToOffsets.find(labelName);
    if(it == mGotoLabelsToOffsets.end())
    {
		int gotoOffset = static_cast<int>(mBytecode.size());
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
		
		#ifdef DEBUG_BUILDER
		std::cout << "Goto Label " << labelName << std::endl;
		#endif
    }
    else
    {
		LogError(loc, StringUtil::Format("label '%s' already exists at (line %d, col %d)",
										 labelName.c_str(), 0, 0));
    }
}

void SheepScriptBuilder::BranchGoto(std::string labelName)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Branch to Goto " << labelName << std::endl;
	#endif
	
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
        auto it2 = mGotoLabelsToBeHookedUp.find(labelName);
        if(it2 == mGotoLabelsToBeHookedUp.end())
        {
            mGotoLabelsToBeHookedUp[labelName] = std::vector<int>();
        }
        mGotoLabelsToBeHookedUp[labelName].push_back(static_cast<int>(mBytecode.size()) - 4);
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

void SheepScriptBuilder::AddSysFuncArg(SheepValue arg, const Location& loc)
{
	#ifdef DEBUG_BUILDER
    std::cout << "SysFunc Arg " << arg.GetTypeString() << std::endl;
    #endif
	mSysFuncArgs.push_back(arg);
}

SheepValueType SheepScriptBuilder::CallSysFunc(std::string sysFuncName, const Location& loc)
{
    #ifdef DEBUG_BUILDER
    std::cout << "SysFunc " << sysFuncName << std::endl;
    #endif
	
	// All possible system functions are pre-registered in a table.
	// Make sure this system function exists!
	SysFunc* sysFunc = GetSysFunc(StringUtil::ToLowerCopy(sysFuncName));
	if(sysFunc == nullptr)
	{
		LogError(loc, "system function '" + sysFuncName + "' not found in export table.");
		mSysFuncArgs.clear();
		return SheepValueType::Void;
	}
	
	// Get expected and actual arg counts for this function.
	// Be sure to reset the arg counter for the next time we try to call a system function!
	int expectedArgCount = static_cast<int>(sysFunc->argumentTypes.size());
	int actualArgCount = static_cast<int>(mSysFuncArgs.size());
	
	// Only let compilation succeed if the number of args passed to the function match the expected number of arguments.
	if(actualArgCount < expectedArgCount)
	{
		LogError(loc, "too few parameters in call to function '" + sysFunc->name + "' (function takes " + std::to_string(expectedArgCount) + " parameters)");
		mSysFuncArgs.clear();
		return SheepValueType::Void;
	}
	else if(actualArgCount > expectedArgCount)
	{
		LogError(loc, "too many parameters in call to function '" + sysFunc->name + "' (function takes " + std::to_string(expectedArgCount) + " parameters)");
		mSysFuncArgs.clear();
		return SheepValueType::Void;
	}
	
	// Check that arg types are compatible.
	bool argTypesCompatible = true;
	for(size_t i = 0; i < sysFunc->argumentTypes.size(); i++)
	{
		SheepValue& value = mSysFuncArgs[i];
		int argType = sysFunc->argumentTypes[i];
		switch(argType)
		{
		case 1: // Int
			if(value.type == SheepValueType::Float)
			{
				LogWarning(loc, StringUtil::Format("float expression loses precision when passed for int parameter %i of function `%s",
												   i+1, sysFuncName.c_str()));
				argTypesCompatible = false;
			}
			else if(value.type == SheepValueType::String)
			{
				LogError(loc, StringUtil::Format("cannot pass string expression for int parameter %i of function `%s`",
												 i+1, sysFuncName.c_str()));
				argTypesCompatible = false;
			}
			break;
			
		case 2: // Float
			if(value.type == SheepValueType::String)
			{
				LogError(loc, StringUtil::Format("cannot pass string expression for float parameter %i of function `%s`",
												 i+1, sysFuncName.c_str()));
				argTypesCompatible = false;
			}
			break;
			
		case 3: // String
			if(value.type == SheepValueType::Int)
			{
				LogError(loc, StringUtil::Format("cannot pass int expression for string parameter %i of function `%s`",
												 i+1, sysFuncName.c_str()));
				argTypesCompatible = false;
			}
			else if(value.type == SheepValueType::Float)
			{
				LogError(loc, StringUtil::Format("cannot pass float expression for string parameter %i of function `%s`",
												 i+1, sysFuncName.c_str()));
				argTypesCompatible = false;
			}
			break;
			
		default:
		case 0:	// Void
			// Void (or anything that's not an int/float/string) are never compatible arg types.
			argTypesCompatible = false;
			break;
		}
	}
	
	// Clear sys func args - don't need them anymore and we want this cleared for next sys func call parse.
	mSysFuncArgs.clear();
	
	// If incompatible argument types, don't go any further.
	if(!argTypesCompatible)
	{
		return SheepValueType::Void;
	}
	
	// Right before calling the function, we want to push the argument
	// count onto the top of the stack.
	PushI(expectedArgCount);
		
	// Add appropriate instruction based on function return type.
	SheepValueType valueType;
	if(sysFunc->returnType == 0)
	{
		#ifdef DEBUG_BUILDER
		std::cout << "CallSysFunctionV" << std::endl;
		#endif
		AddInstruction(SheepInstruction::CallSysFunctionV);
		valueType = SheepValueType::Void;
	}
	else if(sysFunc->returnType == 1)
	{
		#ifdef DEBUG_BUILDER
		std::cout << "CallSysFunctionI" << std::endl;
		#endif
		AddInstruction(SheepInstruction::CallSysFunctionI);
		valueType = SheepValueType::Int;
	}
	else if(sysFunc->returnType == 2)
	{
		#ifdef DEBUG_BUILDER
		std::cout << "CallSysFunctionF" << std::endl;
		#endif
		AddInstruction(SheepInstruction::CallSysFunctionF);
		valueType = SheepValueType::Float;
	}
	else if(sysFunc->returnType == 3)
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
	// Note that this "slices" the SysFunc down to just a SysFuncImport, but that's OK!
	if(sysFuncIndex == mSysImports.size())
	{
		mSysImports.push_back(*sysFunc);
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

void SheepScriptBuilder::BeginIfElseBlock()
{
	#ifdef DEBUG_BUILDER
	std::cout << "BeginIfElseBlock" << std::endl;
	#endif
	mOpenIfBlocks.emplace_back();
}

void SheepScriptBuilder::EndIfElseBlock()
{
	#ifdef DEBUG_BUILDER
	std::cout << "EndIfElseBlock" << std::endl;
	#endif
	
	// At the end of each "if" and "else if" block, we have been putting "branch" statements with placeholder addresses.
	// The address there should be the address of the end of the if/else block...which is right here!
	// So, go back and hook those addresses up.
	int branchOffset = static_cast<int>(mBytecode.size());
	for(auto& offset : mOpenIfBlocks.back().branchAddressOffsets)
	{
		mBytecode[offset] = (branchOffset & 0xFF);
		mBytecode[offset + 1] = (branchOffset >> 8) & 0xFF;
		mBytecode[offset + 2] = (branchOffset >> 16) & 0xFF;
		mBytecode[offset + 3] = (branchOffset >> 24) & 0xFF;
	}
	
	// Pop the open if block - it's not open any longer.
	mOpenIfBlocks.pop_back();
}

void SheepScriptBuilder::BeginIfBlock()
{
	// This is beginning of an if block within a larger if/else block.
	// Ex: if(blah$ == 10)
	// Add branch if zero instruction. This branches PAST the if statement if the conditions is false.
	#ifdef DEBUG_BUILDER
	std::cout << "BranchIfZero, XXX" << std::endl;
	#endif
	AddInstruction(SheepInstruction::BranchIfZero);
	
	// Argument is address to branch to. BUT we don't know this address yet!
	// We just reserve the space and note that we need to circle back to this!
	AddIntArg(-1);
	
	// Record that we need to circle back and fill in the correct branch address.
	mOpenIfBlocks.back().branchIfZeroAddressOffset = static_cast<int>(mBytecode.size()) - 4;
}

void SheepScriptBuilder::EndIfBlock()
{
	// This is end of an if block. We will need to branch past any other "else if" or "else blocks!
	// Add branch instruction.
	#ifdef DEBUG_BUILDER
	std::cout << "Branch, XXX" << std::endl;
	#endif
	AddInstruction(SheepInstruction::Branch);
	
	// Argument is address to branch to. BUT we don't know this address yet!
	// We just reserve the space and note that we need to circle back to this!
	AddIntArg(-1);
	
	// Record that we need to circle back and fill in the correct branch address.
	mOpenIfBlocks.back().branchAddressOffsets.push_back(static_cast<int>(mBytecode.size()) - 4);
	
	// If there was a previous "branch if zero" address to be hooked up (from start of if block), it should point HERE!
	// This effectively has the code execute the NEXT "else if" or "else" statement.
	if(mOpenIfBlocks.back().branchIfZeroAddressOffset > 0)
	{
		int offset = mOpenIfBlocks.back().branchIfZeroAddressOffset;
		int branchOffset = static_cast<int>(mBytecode.size());
		mBytecode[offset] = (branchOffset & 0xFF);
		mBytecode[offset + 1] = (branchOffset >> 8) & 0xFF;
		mBytecode[offset + 2] = (branchOffset >> 16) & 0xFF;
		mBytecode[offset + 3] = (branchOffset >> 24) & 0xFF;
		
		mOpenIfBlocks.back().branchIfZeroAddressOffset = -1;
	}
}

void SheepScriptBuilder::BeginElseBlock()
{
	#ifdef DEBUG_BUILDER
	std::cout << "BeginElseBlock" << std::endl;
	#endif
	// Maybe nothing?
}

void SheepScriptBuilder::EndElseBlock()
{
	#ifdef DEBUG_BUILDER
	std::cout << "EndElseBlock" << std::endl;
	#endif
	// Maybe nothing?
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

void SheepScriptBuilder::Store(std::string varName, const Location& loc)
{
    auto it = mVariableIndexByName.find(varName);
    if(it != mVariableIndexByName.end())
    {
        int varIndex = it->second;
        if(varIndex >= 0 && varIndex < mVariables.size())
        {
            SheepValue& value = mVariables[varIndex];
			//TODO: If trying to assign invalid type, throw error. (need to pass in expr SheepValue to do this)
			//TODO: cannot assign <type> expression to <type> variable '<varName>'
            if(value.type == SheepValueType::Int)
            {
				#ifdef DEBUG_BUILDER
				std::cout << "StoreI, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::StoreI);
            }
            else if(value.type == SheepValueType::Float)
            {
				#ifdef DEBUG_BUILDER
				std::cout << "StoreF, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::StoreF);
            }
            else if(value.type == SheepValueType::String)
            {
				#ifdef DEBUG_BUILDER
				std::cout << "StoreS, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::StoreS);
            }
            AddIntArg(varIndex);
        }
    }
	else
	{
		LogError(loc, StringUtil::Format("user identifier '%s' not found in symbol definitions", varName.c_str()));
	}
}

SheepValueType SheepScriptBuilder::Load(std::string varName, const Location& loc)
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
				#ifdef DEBUG_BUILDER
				std::cout << "LoadI, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::LoadI);
            }
            else if(value.type == SheepValueType::Float)
            {
				#ifdef DEBUG_BUILDER
				std::cout << "LoadF, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::LoadF);
            }
            else if(value.type == SheepValueType::String)
            {
				#ifdef DEBUG_BUILDER
				std::cout << "LoadS, " << varIndex << std::endl;
				#endif
                AddInstruction(SheepInstruction::LoadS);
            }
            AddIntArg(varIndex);
            return value.type;
        }
    }
	else
	{
		LogError(loc, StringUtil::Format("user identifier '%s' not found in symbol definitions", varName.c_str()));
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

SheepValueType SheepScriptBuilder::Add(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Add" << std::endl;
	#endif
	
	// Different results depending on whether both vals are ints, floats, or both.
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed with operator '+'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '+'");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::Subtract(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Subtract" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed with operator '-'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '-'");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::Multiply(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Multiply" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed with operator '*'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '*'");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::Divide(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Divide" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed with operator '/'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '/'");
    }
	return SheepValueType::Void;
}

void SheepScriptBuilder::Negate(SheepValue val, const Location& loc)
{
    if(val.type == SheepValueType::Int)
    {
		#ifdef DEBUG_BUILDER
		std::cout << "NegateI" << std::endl;
		#endif
        AddInstruction(SheepInstruction::NegateI);
    }
    else if(val.type == SheepValueType::Float)
    {
		#ifdef DEBUG_BUILDER
		std::cout << "NegateF" << std::endl;
		#endif
        AddInstruction(SheepInstruction::NegateF);
    }
    else
    {
        LogError(loc, StringUtil::Format("'-' operator not allowed on %s expressions", val.GetTypeString().c_str()));
    }
}

SheepValueType SheepScriptBuilder::IsEqual(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsEqual" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '=='");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '=='");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::IsNotEqual(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsNotEqual" << std::endl;
	#endif
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
	else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '!='");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '!='");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::IsGreater(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsGreater" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '>'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '>'");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::IsLess(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsLess" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '<'");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '<'");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::IsGreaterEqual(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsGreaterEqual" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '>='");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '>='");
    }
	return SheepValueType::Void;
}

SheepValueType SheepScriptBuilder::IsLessEqual(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IsLessEqual" << std::endl;
	#endif
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
    else if(val1.type == SheepValueType::String || val2.type == SheepValueType::String)
	{
		LogError(loc, "string types not allowed for operator '<='");
	}
	else
    {
        LogError(loc, "cannot use void return with operator '<='");
    }
	return SheepValueType::Void;
}

void SheepScriptBuilder::IToF(int index)
{
	#ifdef DEBUG_BUILDER
	std::cout << "IToF, " << index << std::endl;
	#endif
    AddInstruction(SheepInstruction::IToF);
    AddIntArg(index);
}

void SheepScriptBuilder::FToI(int index)
{
	#ifdef DEBUG_BUILDER
	std::cout << "FToI, " << index << std::endl;
	#endif
    AddInstruction(SheepInstruction::FToI);
    AddIntArg(index);
}

void SheepScriptBuilder::Modulo(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Modulo" << std::endl;
	#endif
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
		LogError(loc, "operator '%' requires both left and right expressions be of integer type");
    }
}

void SheepScriptBuilder::And(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "And" << std::endl;
	#endif
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
        LogError(loc, "operator '&&' requires both left and right expressions be of integer type");
    }
}

void SheepScriptBuilder::Or(SheepValue val1, SheepValue val2, const Location& loc)
{
	#ifdef DEBUG_BUILDER
	std::cout << "Or" << std::endl;
	#endif
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
        LogError(loc, "operator '||' requires both left and right expressions be of integer type");
    }
}

void SheepScriptBuilder::Not()
{
	#ifdef DEBUG_BUILDER
    std::cout << "Not" << std::endl;
    #endif
    AddInstruction(SheepInstruction::Not);
}

void SheepScriptBuilder::Breakpoint()
{
	#ifdef DEBUG_BUILDER
	std::cout << "DebugBreakpoint" << std::endl;
	#endif
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
    unsigned char* array;
    array = reinterpret_cast<unsigned char*>(&arg);
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

void SheepScriptBuilder::LogWarning(const Location& loc, const std::string& message)
{
    if(mCompiler != nullptr)
    {
        mCompiler->Warning(this, loc, message);
    }
}

void SheepScriptBuilder::LogError(const Location& loc, const std::string& message)
{
    if(mCompiler != nullptr)
    {
        mCompiler->Error(this, loc, message);
    }
}
