//
// Clark Kromenaker
//
// Manager for compiling and executing sheep scripts.
// Handles complexities of async callbacks, waiting, multithreading, etc.
//
#pragma once
#include <stack>

#include "SheepCompiler.h"
#include "SheepVM.h"

class SheepManager
{
public:
    // Compilation - convert text-based SheepScript to a compiled SheepScript.
    SheepScript* Compile(const char* filePath);
    SheepScript* Compile(const std::string& name, const std::string& sheep);
    SheepScript* Compile(const std::string& name, std::istream& stream);
	
    // Execution - execute a compiled SheepScript.
	void Execute(SheepScript* script, std::function<void()> finishCallback, const std::string& tag = "");
	void Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag = "");

    // Evaluation - special form of SheepScript; only boolean logic is allowed, must evaluate to true or false. Waiting/callbacks are not allowed.
    SheepScript* CompileEval(const std::string& sheep);
    bool Evaluate(SheepScript* script);
	bool Evaluate(SheepScript* script, int n, int v);

    // VM Manipulation
    void StopExecution(const std::string& tag) { mVirtualMachine.StopExecution(tag); }
    void FlagExecutionError() { mVirtualMachine.FlagExecutionError(); }

    // VM State Queries
	SheepThread* GetCurrentThread() const { return mVirtualMachine.GetCurrentThread(); }
	bool IsAnyThreadRunning() const { return mVirtualMachine.IsAnyThreadRunning(); }
	
private:
	// Executes binary bytecode sheep scripts.
	SheepVM mVirtualMachine;
};

extern SheepManager gSheepManager;
