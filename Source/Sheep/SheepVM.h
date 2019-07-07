//
// SheepVM.h
//
// Clark Kromenaker
//
// A virtual machine for executing Sheep bytecode.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Value.h"

class SheepScript;
class SysImport;

enum class SheepValueType
{
    Void,
    Int,
    Float,
    String
};

struct SheepValue
{
    SheepValueType type = SheepValueType::Int;
    union
    {
        int intValue;
        float floatValue;
        const char* stringValue;
    };
    
    SheepValue() { }
    SheepValue(SheepValueType t) { type = t; }
    SheepValue(int i) { type = SheepValueType::Int; intValue = i; }
    SheepValue(float f) { type = SheepValueType::Float; floatValue = f; }
    SheepValue(const char* s) { type = SheepValueType::String; stringValue = s; }
	~SheepValue() { }
};

// GK3 calls these "Object Code" instances.
// Basically, a loaded instance of a sheep script with variables and such.
struct SheepInstance
{
	SheepScript* mSheepScript = nullptr;
	uint32_t mLastUsedTimeMs = 0;
	int mReferenceCount = 0;
	
	// Instanced variables from the sheep script.
	// These'll likely be modified during execution.
	std::vector<SheepValue> mVariables;
	
	std::string GetName();
};

struct SheepThread;

// Notify Links?

enum class SheepInstruction
{
    SitnSpin            = 0x00,
    Yield               = 0x01,
    CallSysFunctionV    = 0x02,
    CallSysFunctionI    = 0x03,
    CallSysFunctionF    = 0x04,
    CallSysFunctionS    = 0x05,
    Branch              = 0x06,
    BranchGoto          = 0x07,
    BranchIfZero        = 0x08,
    BeginWait           = 0x09,
    EndWait             = 0x0A,
    ReturnV             = 0x0B,
	//Unknown1          = 0x0C, // May be "Export" instruction; mentioned in docs as deprecated.
    StoreI              = 0x0D,
    StoreF              = 0x0E,
    StoreS              = 0x0F,
    LoadI               = 0x10,
    LoadF               = 0x11,
    LoadS               = 0x12,
    PushI               = 0x13,
    PushF               = 0x14,
    PushS               = 0x15,
    Pop                 = 0x16,
    AddI                = 0x17,
    AddF                = 0x18,
    SubtractI           = 0x19,
    SubtractF           = 0x1A,
    MultiplyI           = 0x1B,
    MultiplyF           = 0x1C,
    DivideI             = 0x1D,
    DivideF             = 0x1E,
    NegateI             = 0x1F,
    NegateF             = 0x20,
    IsEqualI            = 0x21,
    IsEqualF            = 0x22,
    IsNotEqualI         = 0x23,
    IsNotEqualF         = 0x24,
    IsGreaterI          = 0x25,
    IsGreaterF          = 0x26,
    IsLessI             = 0x27,
    IsLessF             = 0x28,
    IsGreaterEqualI     = 0x29,
    IsGreaterEqualF     = 0x2A,
    IsLessEqualI        = 0x2B,
    IsLessEqualF        = 0x2C,
    IToF                = 0x2D,
    FToI                = 0x2E,
    Modulo              = 0x2F,
    And                 = 0x30,
    Or                  = 0x31,
    Not                 = 0x32,
    GetString           = 0x33,
    DebugBreakpoint     = 0x34
};

class SheepVM
{
	friend class SheepThread;
public:
    SheepVM() { }
	~SheepVM();
    
    void Execute(SheepScript* script);
	void Execute(SheepScript* script, std::function<void()> finishCallback);
	
    void Execute(SheepScript* script, std::string functionName);
	void Execute(SheepScript* script, std::string functionName, std::function<void()> finishCallback);
	
	void Execute(SheepScript* script, int bytecodeOffset, std::function<void()> finishCallback);
	
    bool Evaluate(SheepScript* script);
	
	SheepThread* GetCurrentThread() const { return mCurrentThread; }
	
	bool IsAnyRunning() const;
	
private:
	// Stack for the VM - holds values that are pushed/popped by instructions.
	// Just like the stack C++ functions use (uhh, kind of).
	// Top of the stack is (mStackSize - 1).
	static const int kMaxStackSize = 1024;
	int mStackSize = 0;
	SheepValue mStack[kMaxStackSize];
	
	std::vector<SheepInstance*> mSheepInstances;
	std::vector<SheepThread*> mSheepThreads;
	
	SheepThread* mCurrentThread = nullptr;
	
	SheepInstance* GetInstance(SheepScript* script);
	SheepThread* GetThread();
	
    Value CallSysFunc(SysImport* sysImport);
	
	void PushStackInt(int val);
	void PushStackFloat(float val);
	void PushStackStrOffset(int val);
	void PushStackStr(const char* str);
	
	SheepValue& GetStack(int index) { return mStack[mStackSize - 1 - index]; }
	void PopStack(int count);
	SheepValue& PopStack();
	SheepValue& PeekStack();
	
	void Execute(SheepThread* thread);
};

//TODO: Move to separate file
struct SheepThread
{
	// Reference to this thread's virtual machine.
	SheepVM* mVirtualMachine = nullptr;
	
	// The sheep attached to this thread.
	SheepInstance* mAttachedSheep = nullptr;
	
	// Current code offset for attached sheep (aka the instruction pointer).
	int mCodeOffset = 0;
	
	// Info about the function being executed (mainly for debugging).
	std::string mFunctionName;
	int mFunctionStartOffset = 0;
	
	// If true, this thread is executing the attached sheep.
	// This is still "true" while "waiting" on something else.
	//TODO: Convert to enum
	bool mRunning = false;
	bool mBlocked = false;
	
	// If set, some other thread is waiting for this thread to complete before continuing.
	// Could happen if some sheep "waits" on a CallSheep system function.
	std::function<void()> mWaitCallback = nullptr;
	
	// The number of "things" this thread is waiting on.
	// When exiting a wait block, thread execution will be blocked until this counter reaches zero.
	int mWaitCounter = 0;
	
	// If true, the thread is executing code inside a wait block.
	// Before exiting the wait block, all waited upon functions must complete.
	bool mInWaitBlock = false;
	
	std::string GetName()
	{
		if(mAttachedSheep != nullptr)
		{
			return mAttachedSheep->GetName() + ":" + mFunctionName;
		}
		return ":" + mFunctionName;
	}
	
	std::function<void()> AddWait()
	{
		if(!mInWaitBlock) { return nullptr; }
		mWaitCounter++;
		return std::bind(&SheepThread::OnWaitCompleted, this);
	}
	
	void OnWaitCompleted()
	{
		assert(mInWaitBlock);
		assert(mWaitCounter > 0);
		mWaitCounter--;
		if(mBlocked && mWaitCounter == 0)
		{
			mVirtualMachine->Execute(this);
		}
	}
	
	//TODO
	// owning layer?
	// int mFunctionOffset = 0;
	// debug info?
	// is preloading only?
	// bool mIsCurrentlyBlocking = false;
};
