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

std::string SheepInstance::GetName()
{
	if(mSheepScript != nullptr)
	{
		return mSheepScript->GetNameNoExtension();
	}
	return "";
}

SheepVM::~SheepVM()
{
	for(auto& instance : mSheepInstances)
	{
		delete instance;
	}
	for(auto& thread : mSheepThreads)
	{
		delete thread;
	}
}

void SheepVM::Execute(SheepScript* script)
{
	// We need a valid script.
	if(script == nullptr) { return; }
	
	// Just default to zero offset (aka the first function in the script).
	Execute(script, 0, nullptr);
}

void SheepVM::Execute(SheepScript* script, std::function<void()> finishCallback)
{
	// We need a valid script.
	if(script == nullptr)
	{
		finishCallback();
		return;
	}
	
	// Just default to zero offset (aka the first function in the script).
	Execute(script, 0, finishCallback);
}

void SheepVM::Execute(SheepScript* script, std::string functionName)
{
	Execute(script, functionName, nullptr);
}

void SheepVM::Execute(SheepScript* script, std::string functionName, std::function<void()> finishCallback)
{
	// We need a valid script.
	if(script == nullptr)
	{
		std::cout << "ERROR: Script is not valid." << std::endl;
		finishCallback();
		return;
	}
	
	// Get bytecode offset for this function. If less than zero,
	// it means the function doesn't exist, and we've got to fail out.
	int functionOffset = script->GetFunctionOffset(functionName);
	if(functionOffset < 0)
	{
		std::cout << "ERROR: Couldn't find function: " << functionName << std::endl;
		finishCallback();
		return;
	}
	
	// Retrieve a sheep instance for this script, or fail.
	SheepInstance* instance = GetInstance(script);
	if(instance == nullptr)
	{
		std::cout << "ERROR: Couldn't allocate SheepInstance." << std::endl;
		finishCallback();
		return;
	}
	
	// Create a sheep thread to perform the execution.
	SheepThread* thread = GetThread();
	thread->mAttachedSheep = instance;
	thread->mWaitCallback = finishCallback;
	thread->mCodeOffset = functionOffset;
	
	// Save function name and start offset, mainly for debug output.
	thread->mFunctionName = functionName;
	thread->mFunctionStartOffset = functionOffset;
	
	// Start the thread of execution.
	//std::cout << "SHEEP EXECUTE START - Stack size is " << mStackSize << std::endl;
	Execute(thread);
	//std::cout << "SHEEP EXECUTE END - Stack size is " << mStackSize << std::endl;
}

void SheepVM::Execute(SheepScript* script, int bytecodeOffset, std::function<void()> finishCallback)
{
	if(script == nullptr) { return; }
	
	// Retrieve a sheep instance for this script, or fail.
	SheepInstance* instance = GetInstance(script);
	if(instance == nullptr) { return; }
	
	// Create a sheep thread to perform the execution.
	SheepThread* thread = GetThread();
	thread->mAttachedSheep = instance;
	thread->mWaitCallback = finishCallback;
	thread->mCodeOffset = bytecodeOffset;
	
	// In this case, we don't know the specific function name, so we just use X$.
	// (Technically, it may be possible to dig through the SheepScript* to figure this out, but do we need to?)
	thread->mFunctionName = "X$";
	thread->mFunctionStartOffset = bytecodeOffset;
	
	// Start the thread of execution.
	//std::cout << "SHEEP EXECUTE START - Stack size is " << mStackSize << std::endl;
	Execute(thread);
	//std::cout << "SHEEP EXECUTE END - Stack size is " << mStackSize << std::endl;
}

bool SheepVM::Evaluate(SheepScript* script)
{
	//std::cout << "SHEEP EVALUATE START - Stack size is " << mStackSize << std::endl;
    // Execute the script, per usual.
    Execute(script);
    
    // If stack is empty, return false.
    if(mStackSize == 0) { return false; }
    
    // Check the top item on the stack and return true or false based on that.
	SheepValue& result = PopStack();
	//std::cout << "SHEEP EVALUATE END - Stack size is " << mStackSize << std::endl;
    if(result.type == SheepValueType::Int)
    {
        return result.intValue != 0;
    }
    else if(result.type == SheepValueType::Float)
    {
        return !Math::AreEqual(result.floatValue, 0.0f);
    }
    else if(result.type == SheepValueType::String)
    {
        std::string str(result.stringValue);
        return !str.empty();
    }

    // Default to false.
    return false;
}

bool SheepVM::IsAnyRunning() const
{
	for(auto& thread : mSheepThreads)
	{
		if(thread->mRunning) { return true; }
	}
	return false;
}

SheepInstance* SheepVM::GetInstance(SheepScript* script)
{
	// If an instance already exists for this sheep, just reuse that one.
	for(auto& instance : mSheepInstances)
	{
		if(instance->mSheepScript == script)
		{
			return instance;
		}
	}
	
	//TODO: Try to reuse an existing, but unused, instance.
	
	// Create a new instance if we have to.
	SheepInstance* instance = new SheepInstance();
	instance->mSheepScript = script;
	
	// Create copy of variables for assignment during execution.
	instance->mVariables = script->GetVariables();
	
	// Add to sheep instances.
	mSheepInstances.push_back(instance);
	//std::cout << mSheepInstances.size() << std::endl;
	return instance;
}

SheepThread* SheepVM::GetThread()
{
	// Recycle a previously used thread, if possible.
	SheepThread* useThread = nullptr;
	for(auto& thread : mSheepThreads)
	{
		if(!thread->mRunning)
		{
			useThread = thread;
			break;
		}
	}
	
	// If needed, create a new thread instead.
	if(useThread == nullptr)
	{
		useThread = new SheepThread();
		useThread->mVirtualMachine = this;
		mSheepThreads.push_back(useThread);
	}
	return useThread;
}

Value SheepVM::CallSysFunc(SysImport* sysImport)
{
	// Retrieve system function declaration for the system function import.
	// We need the full declaration to know whether this is a waitable function!
	SysFuncDecl* sysFunc = GetSysFuncDecl(sysImport);
	if(sysFunc == nullptr)
	{
		std::cout << "Sheep uses undeclared function " << sysImport->name << std::endl;
		return Value(0);
	}
	
	// Number on top of stack is argument count.
	// Make sure it matches the argument count from the system function declaration.
	int argCount = PopStack().intValue;
	assert(argCount == sysFunc->argumentTypes.size());
	
	// Retrieve the arguments, of the expected types, from the stack.
	std::vector<Value> args;
	for(int i = 0; i < argCount; i++)
	{
		SheepValue sheepValue = mStack[mStackSize - (argCount - i)];
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
				args.push_back(std::string(sheepValue.stringValue));
				break;
			default:
				std::cout << "Invalid arg type: " << argType << std::endl;
				break;
		}
	}
	PopStack(argCount);
	
	// Based on argument count, call the appropriate function variant.
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

void SheepVM::PushStackInt(int val)
{
	mStackSize++;
	assert(mStackSize < kMaxStackSize);
	mStack[mStackSize - 1].type = SheepValueType::Int;
	mStack[mStackSize - 1].intValue = val;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
}

void SheepVM::PushStackFloat(float val)
{
	mStackSize++;
	assert(mStackSize < kMaxStackSize);
	mStack[mStackSize - 1].type = SheepValueType::Float;
	mStack[mStackSize - 1].intValue = val;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
}

void SheepVM::PushStackStrOffset(int val)
{
	mStackSize++;
	assert(mStackSize < kMaxStackSize);
	mStack[mStackSize - 1].type = SheepValueType::String;
	mStack[mStackSize - 1].intValue = val;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
}

void SheepVM::PushStackStr(const char* str)
{
	mStackSize++;
	assert(mStackSize < kMaxStackSize);
	mStack[mStackSize - 1].type = SheepValueType::String;
	mStack[mStackSize - 1].stringValue = str;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
}

SheepValue& SheepVM::PopStack()
{
	SheepValue& top = mStack[mStackSize - 1];
	mStackSize--;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Pop 1 (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
	assert(mStackSize >= 0); // Or clamp?
	return top;
}

void SheepVM::PopStack(int count)
{
	mStackSize -= count;
	#ifdef SHEEP_DEBUG
	std::cout << "SHEEP STACK: Pop " << count << " (Stack Size = " << mStackSize << ")" << std::endl;
	#endif
	assert(mStackSize >= 0); // Or clamp?
}

SheepValue& SheepVM::PeekStack()
{
	return mStack[mStackSize - 1];
}

void SheepVM::Execute(SheepThread* thread)
{
	// Store previous thread and set passed in thead as the currently executing thread.
	SheepThread* prevThread = mCurrentThread;
	mCurrentThread = thread;
	
	// Sheep is either being created/started, or was released from a wait block.
	if(!thread->mRunning)
	{
		thread->mRunning = true;
		std::cout << "Sheep " << thread->GetName() << " created and starting" << std::endl;
	}
	else if(thread->mInWaitBlock)
	{
		thread->mBlocked = false;
		thread->mInWaitBlock = false;
		std::cout << "Sheep " << thread->GetName() << " released at line -1" << std::endl;
	}
	
	// Get instance/script we'll be using.
	SheepInstance* instance = thread->mAttachedSheep;
	SheepScript* script = instance->mSheepScript;
	
    // Get bytecode and generate a binary reader for easier parsing.
    char* bytecode = script->GetBytecode();
    int bytecodeLength = script->GetBytecodeLength();
    
    // Create reader for the bytecode.
    BinaryReader reader(bytecode, bytecodeLength);
    if(!reader.CanRead()) { return; }
    
    // Skip ahead to desired offset.
    reader.Skip(thread->mCodeOffset);
    
    // Read each byte in turn, interpret and execute the instruction.
	bool stopReading = false;
	while(!reader.IsEof() && !stopReading)
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
                // Not totally sure what this instruction does.
				// Maybe it yields sheep execution until next frame?
				#ifdef SHEEP_DEBUG
				std::cout << "Yield" << std::endl;
				#endif
				stopReading = true;
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
				
				// Execute the system function.
                Value value = CallSysFunc(sysFunc);
				
				// Though this is void return, we still push type of "shpvoid" onto stack.
				// The compiler generates an extra "Pop" instruction after a CallSysFunctionV.
				// This matches how the original game's compiler generated instructions!
				PushStackInt(value.to<shpvoid>());
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
				
				// Execute the system function.
                Value value = CallSysFunc(sysFunc);
				
				// Push the int result onto the stack.
				PushStackInt(value.to<int>());
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
				
				// Execute the system function.
                Value value = CallSysFunc(sysFunc);
				
				// Push the float result onto the stack.
				PushStackFloat(value.to<float>());
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
				
				// Execute the system function.
                Value value = CallSysFunc(sysFunc);
				
				// Push the string result onto the stack.
				PushStackStr(value.to<std::string>().c_str()); //TODO: Seems like this could cause problems? Where is value's string coming from? What if it is deallocated???
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
				// This operation also pops off the stack.
				if(PopStack().intValue == 0)
				{
					reader.Seek(branchAddress);
				}
                break;
            }
            case SheepInstruction::BeginWait:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "BeginWait" << std::endl;
				#endif
				thread->mInWaitBlock = true;
                break;
            }
            case SheepInstruction::EndWait:
            {
				#ifdef SHEEP_DEBUG
				std::cout << "EndWait " << thread->mInWaitBlock << ", " << thread->mWaitCounter << std::endl;
				#endif
				// If waiting on one or more WAIT-able functions, we need to STOP thread execution for now!
				// We will resume this thread's execution once we get enough wait callbacks.
				if(thread->mWaitCounter > 0)
				{
					thread->mBlocked = true;
					stopReading = true;
				}
				else
				{
					thread->mInWaitBlock = false;
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
				thread->mRunning = false;
				stopReading = true;
				break;
            }
            case SheepInstruction::StoreI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreI " << mStack[mStackSize - 1].intValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::Int);
					SheepValue& value = PopStack();
					instance->mVariables[varIndex].intValue = value.intValue;
                }
                break;
            }
            case SheepInstruction::StoreF:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreF " << mStack[mStackSize - 1].floatValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::Float);
					SheepValue& value = PopStack();
                    instance->mVariables[varIndex].floatValue = value.floatValue;
                }
                break;
            }
            case SheepInstruction::StoreS:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "StoreS " << mStack[mStackSize - 1].stringValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::String);
					SheepValue& value = PopStack();
                    instance->mVariables[varIndex].stringValue = value.stringValue;
                }
                break;
            }
            case SheepInstruction::LoadI:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "LoadI " << instance->mVariables[varIndex].intValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::Int);
					PushStackInt(instance->mVariables[varIndex].intValue);
                }
                break;
            }
            case SheepInstruction::LoadF:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "LoadF " << instance->mVariables[varIndex].floatValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::Float);
					PushStackFloat(instance->mVariables[varIndex].floatValue);
                }
                break;
            }
            case SheepInstruction::LoadS:
            {
                int varIndex = reader.ReadInt();
                if(varIndex >= 0 && varIndex < instance->mVariables.size())
                {
					#ifdef SHEEP_DEBUG
					std::cout << "LoadS " << instance->mVariables[varIndex].stringValue << std::endl;
					#endif
					
                    assert(instance->mVariables[varIndex].type == SheepValueType::String);
					PushStackStr(instance->mVariables[varIndex].stringValue);
                }
                break;
            }
            case SheepInstruction::PushI:
            {
                int int1 = reader.ReadInt();
				#ifdef SHEEP_DEBUG
				std::cout << "PushI " << int1 << std::endl;
				#endif
				PushStackInt(int1);
                break;
            }
            case SheepInstruction::PushF:
            {
                float float1 = reader.ReadFloat();
				#ifdef SHEEP_DEBUG
				std::cout << "PushF " << float1 << std::endl;
				#endif
				PushStackFloat(float1);
                break;
            }
            case SheepInstruction::PushS:
            {
                int stringConstOffset = reader.ReadInt();
				#ifdef SHEEP_DEBUG
				std::cout << "PushS " << stringConstOffset << std::endl;
				#endif
				PushStackStrOffset(stringConstOffset);
                break;
            }
			case SheepInstruction::GetString:
			{
				SheepValue& offsetValue = PopStack();
				std::string* stringPtr = script->GetStringConst(offsetValue.intValue);
				if(stringPtr != nullptr)
				{
					PushStackStr(stringPtr->c_str());
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
				PopStack(1);
                break;
            }
            case SheepInstruction::AddI:
            {
                if(mStackSize < 2) { break; }
				
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "AddI " << int1 << " + " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 + int2);
                break;
            }
            case SheepInstruction::AddF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "AddF " << float1 << " + " << float2 << std::endl;
				#endif
				
				PushStackFloat(float1 + float2);
                break;
            }
            case SheepInstruction::SubtractI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "SubtractI " << int1 << " - " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 - int2);
                break;
            }
            case SheepInstruction::SubtractF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "SubtractF " << float1 << " - " << float2 << std::endl;
				#endif
				
				PushStackFloat(float1 - float2);
                break;
            }
            case SheepInstruction::MultiplyI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "MultiplyI " << int1 << " * " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 * int2);
                break;
            }
            case SheepInstruction::MultiplyF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "MultiplyF " << float1 << " * " << float2 << std::endl;
				#endif
				
				PushStackFloat(float1 * float2);
                break;
            }
            case SheepInstruction::DivideI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "DivideI " << int1 << " / " << int2 << std::endl;
				#endif
				
				// If dividing by zero, we'll spit out an error and just put a zero on the stack.
				if(int2 != 0)
				{
					PushStackInt(int1 / int2);
				}
				else
				{
					std::cout << "Divide by zero!" << std::endl;
					PushStackInt(0);
				}
                break;
            }
            case SheepInstruction::DivideF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "DivideF " << float1 << " / " << float2 << std::endl;
				#endif
				
				// If dividing by zero, we'll spit out an error and just put a zero on the stack.
				if(!Math::AreEqual(float2, 0.0f))
				{
					PushStackFloat(float1 / float2);
				}
				else
				{
					std::cout << "Divide by zero!" << std::endl;
					PushStackFloat(0);
				}
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
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsEqualI " << int1 << " == " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 == int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsEqualF " << float1 << " == " << float2 << std::endl;
				#endif
				
				PushStackInt(Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsNotEqualI " << int1 << " != " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 != int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsNotEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsNotEqualF " << float1 << " != " << float2 << std::endl;
				#endif
				
				PushStackInt(!Math::AreEqual(float1, float2) ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterI " << int1 << " > " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 > int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterF " << float1 << " > " << float2 << std::endl;
				#endif
				
				PushStackInt(float1 > float2 ? 1 : 0);
                break;
            }
			case SheepInstruction::IsLessI:
			{
				if(mStackSize < 2) { break; }
				
				int int1 = mStack[mStackSize - 2].intValue;
				int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessI " << int1 << " < " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 < int2 ? 1 : 0);
				break;
			}
			case SheepInstruction::IsLessF:
			{
				if(mStackSize < 2) { break; }
				
				float float1 = mStack[mStackSize - 2].floatValue;
				float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessF " << float1 << " < " << float2 << std::endl;
				#endif
				
				PushStackInt(float1 < float2 ? 1 : 0);
				break;
			}
            case SheepInstruction::IsGreaterEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterEqualI " << int1 << " >= " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 >= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsGreaterEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsGreaterEqualF " << float1 << " >= " << float2 << std::endl;
				#endif
				
				PushStackInt(float1 >= float2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualI:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessEqualI " << int1 << " <= " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 <= int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::IsLessEqualF:
            {
                if(mStackSize < 2) { break; }
                
                float float1 = mStack[mStackSize - 2].floatValue;
                float float2 = mStack[mStackSize - 1].floatValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "IsLessEqualF " << float1 << " <= " << float2 << std::endl;
				#endif
				
				PushStackInt(float1 <= float2 ? 1 : 0);
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
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "Modulo " << int1 << " % " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 % int2);
                break;
            }
            case SheepInstruction::And:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "And " << int1 << " && " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 && int2 ? 1 : 0);
                break;
            }
            case SheepInstruction::Or:
            {
                if(mStackSize < 2) { break; }
                
                int int1 = mStack[mStackSize - 2].intValue;
                int int2 = mStack[mStackSize - 1].intValue;
				PopStack(2);
				#ifdef SHEEP_DEBUG
				std::cout << "Or " << int1 << " || " << int2 << std::endl;
				#endif
				
				PushStackInt(int1 || int2 ? 1 : 0);
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
				//TODO: Break in Xcode/VS.
                break;
            }
            default:
            {
				std::cout << "Unaccounted for Sheep Instruction: " << (int)instruction << std::endl;
                break;
            }
        }
    }
	
	// Update thread's code offset value.
	thread->mCodeOffset = reader.GetPosition();
	
	// If reached end of file, assume the thread is no longer running.
	if(reader.IsEof())
	{
		thread->mRunning = false;
	}
	
	// If thread is no longer running, notify anyone who was waiting for the thread to finish.
	// If we get here and the thread IS running, it means the thread was blocked due to a wait!
	if(!thread->mRunning)
	{
		std::cout << "Sheep " << thread->GetName() << " is exiting" << std::endl;
		if(thread->mWaitCallback)
		{
			thread->mWaitCallback();
		}
	}
	else if(thread->mInWaitBlock)
	{
		std::cout << "Sheep " << thread->GetName() << " is blocked at line -1" << std::endl;
	}
	else
	{
		std::cout << "Sheep " << thread->GetName() << " is in some weird unexpected state!" << std::endl;
	}
	
	// Restore previously executing thread.
	mCurrentThread = prevThread;
}
