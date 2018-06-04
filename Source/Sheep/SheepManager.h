//
// SheepManager.h
//
// Clark Kromenaker
//
// Manager for compiling and executing sheep scripts.
// Handles complexities of async callbacks, waiting, multithreading, etc.
//
#pragma once
#include "SheepVM.h"
#include "SheepCompiler.h"

class SheepManager
{
public:
    SheepScript* Compile(const char* filename);
    SheepScript* Compile(std::string sheep);
    SheepScript* Compile(std::istream& stream);
    
    void Execute(std::string assetName, std::string functionName);
    
    void Execute(SheepScript* script);
    bool Evaluate(SheepScript* script);
    
private:
    SheepCompiler mCompiler;
    //SheepVM mVirtualMachine;
};
