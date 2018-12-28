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
    
    void Execute(std::string assetName, std::string functionName);
    void Execute(SheepScript* script);
	void Execute(SheepScript* script, std::string functionName);
	void Execute(std::string functionName);
	
    bool Evaluate(SheepScript* script);
	
	SheepScript* GetCurrentSheepScript() const;
    
private:
    SheepCompiler mCompiler;
	
	// We sometimes need to know what the current sheep script is.
	// This can be complicated by sheep scripts calling each other.
	// This stack helps us keep that all in order.
	std::stack<SheepScript*> mSheepScriptStack;
};
