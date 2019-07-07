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
    SheepScript* Compile(const char* filename);
    SheepScript* Compile(std::string sheep);
    SheepScript* Compile(std::istream& stream);
    
    void Execute(std::string sheepName, std::string functionName);
	void Execute(std::string sheepName, std::string functionName, std::function<void()> finishCallback);
	
    void Execute(SheepScript* script);
	void Execute(SheepScript* script, std::function<void()> finishCallback);
	
	void Execute(SheepScript* script, std::string functionName);
	void Execute(SheepScript* script, std::string functionName, std::function<void()> finishCallback);
	
    bool Evaluate(SheepScript* script);
	
	SheepThread* GetCurrentThread() const { return mVirtualMachine.GetCurrentThread(); }
	
	bool IsAnyRunning() const { return mVirtualMachine.IsAnyRunning(); }
	
private:
	// Compiles text-based sheep script into sheep bytecode, represented as a SheepScript asset.
    SheepCompiler mCompiler;
	
	// Executes binary bytecode sheep scripts.
	SheepVM mVirtualMachine;
};
