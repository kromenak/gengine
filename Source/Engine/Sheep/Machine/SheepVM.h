//
// Clark Kromenaker
//
// A virtual machine for executing Sheep bytecode.
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <iostream>

#include "Profiler.h"
#include "SheepThread.h"
#include "SheepValue.h"
#include "Value.h"

class PersistState;
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

// Allows a SheepThread to wait for a callback, but also allows it to detach if it no longer cares.
// Ex: say a SheepThread starts a long-running timer, BUT is stopped prematurely.
// Ex: The timer callback will still occur - we can't control that anymore. But using a notify link allows us to ignore the result.
struct NotifyLink
{
    // The SheepThread, if any, associated with this instance.
    // If null, any OnNotify call will have no effect.
    SheepThread* thread = nullptr;

    // State of this instance.
    // If waiting for a callback, this is false. If callback was received, this is true.
    bool notified = true;

    // This can be helpful to debug unexpectedly long callbacks.
    //int id = 0;
    //Stopwatch stopwatch;

    std::function<void()> AddNotify();
    void OnNotify();
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
    friend struct NotifyLink;
public:
    SheepVM() = default;
    ~SheepVM();

    SheepThreadId Execute(SheepScript* script, std::function<void()> finishCallback, const std::string& tag = "");
    SheepThreadId Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag = "");

    bool Evaluate(SheepScript* script, int n, int v);

    void StopExecution(const std::string& tag);
    void FlagExecutionError() { mExecutionError = true; }

    SheepThread* GetCurrentThread() const { return mCurrentThread; }
    bool IsAnyThreadRunning() const;
    bool IsThreadRunning(SheepThreadId id) const;

    void OnPersist(PersistState& ps);

private:
    // Each time we execute a Sheepscript, we assign the execution thread a unique ID.
    // This stores what the next ID should be.
    SheepThreadId mNextExecutionId = 1;

    // Instances of SheepScripts that have been created for execution.
    std::vector<SheepInstance*> mSheepInstances;

    // Threads that have been created for executing SheepScript instances.
    std::vector<SheepThread*> mSheepThreads;

    // Notify links that have been created.
    std::vector<NotifyLink*> mNotifyLinks;

    // The thread that is currently executing, if any.
    SheepThread* mCurrentThread = nullptr;

    // If true, the current Sheep thread has encountered an execution error.
    bool mExecutionError = false;

    SheepInstance* GetInstance(SheepScript* script);
    SheepThread* GetIdleThread();
    NotifyLink* GetNotifyLink();

    Value CallSysFunc(SheepThread* thread, SysFuncImport* sysImport);

    SheepThread* CreateThread(SheepInstance* instance, int bytecodeOffset, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag);
    SheepThread* StartExecution(SheepInstance* instance, int bytecodeOffset, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag);
    void ContinueExecution(SheepThread* thread);
};
