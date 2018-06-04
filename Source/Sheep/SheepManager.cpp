//
// SheepManager.cpp
//
// Clark Kromenaker
//
#include "SheepManager.h"

SheepScript* SheepManager::Compile(const char *filename)
{
    return mCompiler.Compile(filename);
}

SheepScript* SheepManager::Compile(std::string sheep)
{
    return mCompiler.Compile(sheep);
}

SheepScript* SheepManager::Compile(std::istream &stream)
{
    return mCompiler.Compile(stream);
}

void SheepManager::Execute(std::string assetName, std::string functionName)
{
    
}

void SheepManager::Execute(SheepScript* script)
{
    // VM currently fails when executing multiple scripts (not re-entrant?)
    // Something to look into/fix in the future.
    SheepVM vm;
    vm.Execute(script);
    //mVirtualMachine.Execute(script);
}

bool SheepManager::Evaluate(SheepScript* script)
{
    SheepVM vm;
    return vm.Evaluate(script);
    //return mVirtualMachine.Evaluate(script);
}
