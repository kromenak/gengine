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
};

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

class SheepScript;
class SysImport;

class SheepVM
{
public:
	static SheepVM* GetCurrent() { return sCurrent; }
	
    SheepVM() { }
    
    void Execute(SheepScript* script);
    void Execute(SheepScript* script, std::string functionName);
    
    bool Evaluate(SheepScript* script);
	
	std::function<void()> GetWaitCallback();
	
private:
	static SheepVM* sCurrent;
	
	// Max stack size supported.
	// May need to increase/decrease to an optimal value.
    static const int kMaxStackSize = 64;
	
	// The currently executing sheep script.
	// Stored to enable pause/resume of execution.
	SheepScript* mSheepScript = nullptr;
	
	// Instanced variables from the sheep script.
	std::vector<SheepValue> mVariables;
	
	// Current stack and stack size.
    int mStackSize = 0;
    SheepValue mStack[kMaxStackSize];
	
	// If true, we are within a "wait" statement or block.
	// If a sheep method is "wait" compatible, the VM will pause until it receives a callback.
	bool mInWaitBlock = false;
	int mWaitCount = 0;
	int mContinueAtOffset = 0;
    
    void Execute(SheepScript* script, int bytecodeOffset);
    
    Value CallSysFunc(SysImport* sysFunc);
	
	void OnWaitCallback();
	
	SheepValue& PopStack();
	SheepValue& PushStack(int val);
	SheepValue& PushStack(float val);
	SheepValue& PushStack(std::string val);
    SheepValue& GetStack(int index) { return mStack[mStackSize - 1 - index]; }
};
