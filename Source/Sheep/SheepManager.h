//
// SheepManager.h
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
    SheepScript* Compile(const char* filePath);
    SheepScript* Compile(const std::string& name, const std::string& sheep);
    SheepScript* Compile(const std::string& name, std::istream& stream);
	SheepScript* CompileEval(const std::string& sheep);
    
	void Execute(const std::string& sheepName, const std::string& functionName, std::function<void()> finishCallback);
	void Execute(SheepScript* script, std::function<void()> finishCallback);
	void Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback);
	
    bool Evaluate(SheepScript* script);
	bool Evaluate(SheepScript* script, int n, int v);
	
	SheepThread* GetCurrentThread() const { return mVirtualMachine.GetCurrentThread(); }
	bool IsAnyRunning() const { return mVirtualMachine.IsAnyRunning(); }
	void FlagExecutionError() { mVirtualMachine.FlagExecutionError(); }
	
private:
	// Compiles text-based sheep script into sheep bytecode, represented as a SheepScript asset.
    SheepCompiler mCompiler;
	
	// Executes binary bytecode sheep scripts.
	SheepVM mVirtualMachine;
	
	std::string mEvalHusk = "symbols { int n$ = 0; int v$ = 0; } code { X$() %s }";
};
