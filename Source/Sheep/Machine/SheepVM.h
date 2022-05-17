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

#include "SheepThread.h"
#include "SheepValue.h"
#include "Value.h"

class SheepScript;
struct SysFuncImport;

// GK3 calls these "Object Code" instances.
// Basically, a loaded instance of a sheep script with variables and such.
struct SheepInstance
{
	// The script being executed in this context.
	SheepScript* mSheepScript = nullptr;
	
	// Instanced variables from the sheep script.
	// These'll likely be modified during execution.
	std::vector<SheepValue> mVariables;
	
	// For debugging, the last time this object was in use during a sheep thread execution.
	uint32_t mLastUsedTimeMs = 0;
	
	// Number of references. Multiple sheep threads can be using the same executing context.
	// For example, if one function calls another in the same SheepScript.
	int mReferenceCount = 0;
	
	std::string GetName();
};

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
    EndWait             = 0x0A,	// 10
    ReturnV             = 0x0B,
	//Unknown1          = 0x0C, // May be "Export" instruction; mentioned in docs as deprecated.
    StoreI              = 0x0D,
    StoreF              = 0x0E,
    StoreS              = 0x0F,
    LoadI               = 0x10,
    LoadF               = 0x11,
    LoadS               = 0x12,
    PushI               = 0x13,
    PushF               = 0x14, // 20
    PushS               = 0x15,
    Pop                 = 0x16,
    AddI                = 0x17,
    AddF                = 0x18,
    SubtractI           = 0x19,
    SubtractF           = 0x1A,
    MultiplyI           = 0x1B,
    MultiplyF           = 0x1C,
    DivideI             = 0x1D,
    DivideF             = 0x1E, // 30
    NegateI             = 0x1F,
    NegateF             = 0x20,
    IsEqualI            = 0x21,
    IsEqualF            = 0x22,
    IsNotEqualI         = 0x23,
    IsNotEqualF         = 0x24,
    IsGreaterI          = 0x25,
    IsGreaterF          = 0x26,
    IsLessI             = 0x27,
    IsLessF             = 0x28, // 40
    IsGreaterEqualI     = 0x29,
    IsGreaterEqualF     = 0x2A,
    IsLessEqualI        = 0x2B,
    IsLessEqualF        = 0x2C,
    IToF                = 0x2D,
    FToI                = 0x2E,
    Modulo              = 0x2F,
    And                 = 0x30,
    Or                  = 0x31,
    Not                 = 0x32, // 50
    GetString           = 0x33,
    DebugBreakpoint     = 0x34
};

class SheepVM
{
	friend struct SheepThread;
public:
	SheepVM() = default;
	~SheepVM();
    
	void Execute(SheepScript* script, std::function<void()> finishCallback);
	void Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback);
	void Execute(SheepScript* script, int bytecodeOffset, std::function<void()> finishCallback);
	
    bool Evaluate(SheepScript* script, int n, int v);
	
	SheepThread* GetCurrentThread() const { return mCurrentThread; }
	bool IsAnyRunning() const;
	
	void FlagExecutionError() { mExecutionError = true; }
	
private:
	std::vector<SheepInstance*> mSheepInstances;
	std::vector<SheepThread*> mSheepThreads;
	
	SheepThread* mCurrentThread = nullptr;
	
	bool mExecutionError = false;
		
	SheepInstance* GetInstance(SheepScript* script);
	SheepThread* GetThread();
	
    Value CallSysFunc(SheepThread* thread, SysFuncImport* sysImport);
	
	SheepThread* ExecuteInternal(SheepScript* script, int bytecodeOffset, const std::string& functionName, std::function<void()> finishCallback);
	SheepThread* ExecuteInternal(SheepInstance* instance, int bytecodeOffset, const std::string& functionName, std::function<void()> finishCallback);
	void ExecuteInternal(SheepThread* thread);
};
